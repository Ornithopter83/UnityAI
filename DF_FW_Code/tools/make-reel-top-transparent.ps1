param(
    [Parameter(Mandatory = $true)]
    [string]$InputPath,

    [Parameter(Mandatory = $true)]
    [string]$OutputPath
)

$source = @'
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

public static class ConnectedNeutralBackground
{
    public static void Remove(string inputPath, string outputPath)
    {
        using (Bitmap input = new Bitmap(inputPath))
        using (Bitmap output = new Bitmap(input.Width, input.Height, PixelFormat.Format32bppArgb))
        {
            using (Graphics graphics = Graphics.FromImage(output))
                graphics.DrawImageUnscaled(input, 0, 0);

            Rectangle area = new Rectangle(0, 0, output.Width, output.Height);
            BitmapData data = output.LockBits(area, ImageLockMode.ReadWrite, PixelFormat.Format32bppArgb);
            int byteCount = Math.Abs(data.Stride) * output.Height;
            byte[] pixels = new byte[byteCount];
            Marshal.Copy(data.Scan0, pixels, 0, byteCount);

            int width = output.Width;
            int height = output.Height;
            bool[] background = new bool[width * height];
            int[] queue = new int[width * height];
            int head = 0;
            int tail = 0;

            for (int x = 0; x < width; x++)
            {
                EnqueueIfBackground(x, 0, width, data, pixels, background, queue, ref tail);
                EnqueueIfBackground(x, height - 1, width, data, pixels, background, queue, ref tail);
            }
            for (int y = 1; y < height - 1; y++)
            {
                EnqueueIfBackground(0, y, width, data, pixels, background, queue, ref tail);
                EnqueueIfBackground(width - 1, y, width, data, pixels, background, queue, ref tail);
            }

            while (head < tail)
            {
                int index = queue[head++];
                int x = index % width;
                int y = index / width;
                if (x > 0) EnqueueIfBackground(x - 1, y, width, data, pixels, background, queue, ref tail);
                if (x + 1 < width) EnqueueIfBackground(x + 1, y, width, data, pixels, background, queue, ref tail);
                if (y > 0) EnqueueIfBackground(x, y - 1, width, data, pixels, background, queue, ref tail);
                if (y + 1 < height) EnqueueIfBackground(x, y + 1, width, data, pixels, background, queue, ref tail);
            }

            for (int index = 0; index < background.Length; index++)
            {
                if (!background[index]) continue;
                int x = index % width;
                int y = index / width;
                pixels[y * data.Stride + x * 4 + 3] = 0;
            }

            Marshal.Copy(pixels, 0, data.Scan0, byteCount);
            output.UnlockBits(data);
            output.Save(outputPath, ImageFormat.Png);
        }
    }

    private static void EnqueueIfBackground(int x, int y, int width, BitmapData data,
        byte[] pixels, bool[] background, int[] queue, ref int tail)
    {
        int index = y * width + x;
        if (background[index]) return;
        int offset = y * data.Stride + x * 4;
        int blue = pixels[offset];
        int green = pixels[offset + 1];
        int red = pixels[offset + 2];
        int minimum = Math.Min(red, Math.Min(green, blue));
        int maximum = Math.Max(red, Math.Max(green, blue));
        if (minimum < 205 || maximum - minimum > 10) return;
        background[index] = true;
        queue[tail++] = index;
    }
}
'@

$drawingAssembly = if ($PSVersionTable.PSEdition -eq 'Desktop') { 'System.Drawing' } else { 'System.Drawing.Common' }
Add-Type -TypeDefinition $source -ReferencedAssemblies $drawingAssembly
[ConnectedNeutralBackground]::Remove($InputPath, $OutputPath)
