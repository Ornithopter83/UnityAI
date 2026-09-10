Shader "FishingGame/TitleWater"
{
    Properties
    {
        _DeepColor ("Deep Color", Color) = (0.0157, 0.0471, 0.3569, 1)
        _SurfaceColor ("Surface Color", Color) = (0, 0.6039, 0.6235, 1)
        _SkyColor ("Sky Reflection", Color) = (0.552, 0.8524, 1, 1)
        _NormalMap ("Wave Normals", 2D) = "bump" {}
        _FoamTex ("Foam", 2D) = "white" {}
        _Smoothness ("Smoothness", Range(0,1)) = 0.74
        _NormalStrength ("Normal Strength", Range(0,2)) = 0.514
        _WaveHeight ("Wave Height", Range(0,2)) = 0.42
        _WaveSpeed ("Wave Speed", Range(0,3)) = 0.65
        _HorizontalChop ("Horizontal Chop", Range(0,2)) = 0.7
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" "Queue"="Geometry-10" }
        LOD 300

        CGPROGRAM
        #pragma surface surf Standard fullforwardshadows vertex:vert
        #pragma target 3.0

        sampler2D _NormalMap;
        sampler2D _FoamTex;
        fixed4 _DeepColor;
        fixed4 _SurfaceColor;
        fixed4 _SkyColor;
        half _Smoothness;
        half _NormalStrength;
        half _WaveHeight;
        half _WaveSpeed;
        half _HorizontalChop;

        struct Input
        {
            float2 uv_NormalMap;
            float3 viewDir;
            float3 worldPos;
        };

        void vert(inout appdata_full v)
        {
            float2 p = v.vertex.xz;
            float t = _Time.y * _WaveSpeed;
            // Original Crest input used heading 0 and strong FFT wind. Negative
            // time in phase space makes the visible crests travel toward +X.
            float primary = sin(p.x * 0.17 - t);
            float cross = cos((p.x * 0.08 + p.y * 0.13) - t * 1.23);
            v.vertex.y += (primary + cross) * _WaveHeight;
            v.vertex.x += (primary * 0.7 + cross * 0.3) * _HorizontalChop;
        }

        void surf(Input IN, inout SurfaceOutputStandard o)
        {
            float2 uv1 = IN.worldPos.xz * 0.025 + float2(-_Time.y * 0.026, _Time.y * 0.004);
            float2 uv2 = IN.worldPos.xz * 0.041 + float2(-_Time.y * 0.017, -_Time.y * 0.006);
            fixed3 n1 = UnpackNormal(tex2D(_NormalMap, uv1));
            fixed3 n2 = UnpackNormal(tex2D(_NormalMap, uv2));
            o.Normal = normalize(lerp(fixed3(0,0,1), normalize(n1 + n2), _NormalStrength));

            half fresnel = pow(1.0 - saturate(dot(normalize(IN.viewDir), o.Normal)), 1.0);
            fixed3 water = lerp(_SurfaceColor.rgb, _DeepColor.rgb, saturate(IN.worldPos.z / 650.0));
            half foam = tex2D(_FoamTex, uv1 * 0.42 + uv2 * 0.18).r;
            half crest = saturate((n1.r + n2.g - 0.78) * 1.8) * foam;
            o.Albedo = lerp(water, _SkyColor.rgb, 0.18 + fresnel * 0.66);
            o.Albedo = lerp(o.Albedo, fixed3(0.92, 0.97, 1.0), crest * 0.34);
            o.Metallic = 0.02;
            o.Smoothness = _Smoothness;
        }
        ENDCG
    }
    FallBack "Diffuse"
}
