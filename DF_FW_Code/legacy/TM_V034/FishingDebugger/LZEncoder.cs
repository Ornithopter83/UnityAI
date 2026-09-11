using System;

static class LZ
{
	const int DICSIZ = 4096;
	const int HASHSIZ = 1001;
	const int MAXLEN = 18;
	const int DEPTH = 1024;

	static int GetKey(byte[] Dic, int r)
	{
		return (Dic[r] ^ (Dic[r + 1] << 4) ^ (Dic[r + 2] << 8)) % HASHSIZ;
	}

	public static int EncodeLZ(byte[] src, byte[] dst, int len, int targetLen)
	{
		if (len <= 24)
			goto NoCompr;

		byte[] Dic = new byte[DICSIZ + MAXLEN];
		short[] Head = new short[HASHSIZ];
		short[] Prev = new short[DICSIZ];
		short[] Count = new short[HASHSIZ];

		int r = 0, s, i;
		int matchpos = 0, matchlen = 0;
		int incount = 0, outcount;
		int filesize = len;
		int remaincode;

		byte[] codebuf = new byte[17];
		byte mask = 0x80;
		int buftail = 1;

		for (i = 0; i < HASHSIZ; i++)
		{
			Head[i] = -1;
			Count[i] = 0;
		}

		// 헤더 기록 (SHORT_LEN)
		dst[0] = (byte)(filesize >> 8);
		dst[1] = (byte)(filesize & 0xFF);
		outcount = 2;

		Array.Clear(Head, 0, HASHSIZ);
		Array.Clear(Count, 0, HASHSIZ);

		i = Math.Min(filesize, MAXLEN);
		Buffer.BlockCopy(src, 0, Dic, 0, i);
		Buffer.BlockCopy(src, 0, Dic, DICSIZ, i);

		incount = s = i;
		remaincode = i;
		codebuf[0] = 0;

		while (true)
		{
			if (remaincode <= 0)
				break;

			if (matchlen > remaincode)
				matchlen = remaincode;

			if (matchlen < 3)
			{
				codebuf[0] |= mask;
				matchlen = 1;
				codebuf[buftail++] = Dic[r];
			}
			else
			{
				codebuf[buftail++] = (byte)(matchpos >> 4);
				codebuf[buftail++] = (byte)((matchpos << 4) | (matchlen - 3));
			}

			if (mask != 1)
			{
				mask >>= 1;
			}
			else
			{
				if ((outcount + buftail) >= filesize ||
					(outcount + buftail) > targetLen)
					goto NoCompr;

				Buffer.BlockCopy(codebuf, 0, dst, outcount, buftail);
				outcount += buftail;

				mask = 0x80;
				buftail = 1;
				codebuf[0] = 0;
			}

			for (i = 0; i < matchlen; i++)
			{
				if (incount >= filesize)
				{
					remaincode--;
				}
				else
				{
					byte code = src[incount++];
					if (incount > DICSIZ)
					{
						int keyDel = GetKey(Dic, s);
						Count[keyDel]--;
						Count[keyDel] = (short)(Count[keyDel] & 0xefff);
					}

					Dic[s] = code;
					if (s < MAXLEN)
						Dic[s + DICSIZ] = code;
				}

				int key = GetKey(Dic, r);
				Prev[r] = Head[key];
				Head[key] = (short)r;
				Count[key]++;

				r++;
				if (r == DICSIZ) r = 0;

				s++;
				if (s == DICSIZ) s = 0;
			}

			// ===== 매칭 탐색 =====
			{
				int key = GetKey(Dic, r);
				matchlen = 2;
				int n = Count[key];
				int pos = Head[key];
				int d = DEPTH;

				while (--n >= 0 && d-- > 0)
				{
					int len2 = 0;
					int rp = r;
					int pp = pos;

					if (Dic[(rp + matchlen) % DICSIZ] ==
						Dic[(pp + matchlen) % DICSIZ])
					{
						while (len2 < MAXLEN &&
							   Dic[rp] == Dic[pp])
						{
							rp++; if (rp == DICSIZ) rp = 0;
							pp++; if (pp == DICSIZ) pp = 0;
							len2++;
						}
					}

					if (len2 > matchlen)
					{
						matchpos = pos;
						matchlen = len2;
						if (len2 >= MAXLEN)
							break;
					}

					pos = Prev[pos];
				}
			}
		}

		if ((outcount + buftail) >= filesize)
			goto NoCompr;

		Buffer.BlockCopy(codebuf, 0, dst, outcount, buftail);
		outcount += buftail;

		return outcount;

	NoCompr:
		Buffer.BlockCopy(src, 0, dst, 0, len);
		return len;
	}
	public static int DecodeLZ(byte[] src, byte[] dst, int opt = 0)
	{
		const int DICSIZ = 4096;

		int r = 0;
		int outcount = 0;
		int mask = 0;
		int flag = 0;

		int filesize = (src[0] << 8) | src[1];

		if (opt != 0 && filesize == opt)
			goto NoCompr;

		int incount = 2;

		byte[] Dic = new byte[DICSIZ];

		while (outcount < filesize)
		{
			if (mask == 0)
			{
				flag = src[incount++];
				mask = 0x80;
			}

			if ((mask & flag) != 0)
			{
				Dic[r] = src[incount++];
				dst[outcount++] = Dic[r];

				if (++r == DICSIZ) r = 0;
			}
			else
			{
				int pos = src[incount++];
				int len = src[incount++];

				pos = ((pos << 8) + len) >> 4;
				len = (len & 0x0F) + 3;

				if (outcount >= filesize)
					goto NoCompr;

				for (int i = 0; i < len; i++, outcount++)
				{
					if (outcount >= filesize)
						goto NoCompr;

					dst[outcount] = Dic[pos];
					Dic[r] = Dic[pos];

					if (++pos == DICSIZ) pos = 0;
					if (++r == DICSIZ) r = 0;
				}
			}

			mask >>= 1;
		}

		return outcount;

	NoCompr:
		Buffer.BlockCopy(src, 0, dst, 0, filesize);
		return filesize;
	}
}