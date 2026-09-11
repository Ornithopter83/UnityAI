using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Globalization;

namespace DFTestModule;

internal sealed class ImuOrientationView : Control
{
    private const float RightScale = 0.90f;
    private const float TopScale = 0.50f;
    private const float FrontScale = 1.20f;
    private const float HandleStepDegrees = 18.0f;

    private readonly Point3[] model;
    private readonly Face[] faces;
    private readonly Point3[] handleModel;
    private readonly Face[] handleFaces;
    private readonly Image? reelTopImage;
    private readonly Image? reelBottomImage;

    private readonly Point3 camera = new(3.2f, 2.4f, 6.5f);
    private readonly Point3 cameraForward;
    private readonly Point3 cameraRight;
    private readonly Point3 cameraUp;
    private Matrix3 sensorOrientation = Matrix3.Identity;
    private Matrix3 zeroOrientation = Matrix3.Identity;
    private float roll;
    private float pitch;
    private float yaw;
    private float zeroRoll;
    private float zeroPitch;
    private float zeroYaw;
    private float handleAngle;
    private bool hasData;
    private bool hasZero;

    public ImuOrientationView()
    {
        BuildReelModel(out model, out faces, out handleModel, out handleFaces);
        reelTopImage = LoadEmbeddedImage("DFTestModule.Assets.reel-top-v2.png");
        reelBottomImage = LoadEmbeddedImage("DFTestModule.Assets.reel-bottom.png");
        DoubleBuffered = true;
        SetStyle(ControlStyles.ResizeRedraw, true);
        BackColor = Color.FromArgb(247, 248, 250);
        cameraForward = Normalize(new Point3(-camera.X, -camera.Y, -camera.Z));
        cameraRight = Normalize(Cross(cameraForward, new Point3(0, 1, 0)));
        cameraUp = Normalize(Cross(cameraRight, cameraForward));
    }

    public void SetAngles(float newRoll, float newPitch, float newYaw)
    {
        roll = newRoll;
        pitch = newPitch;
        yaw = newYaw;
        sensorOrientation = CreateOrientation(roll, pitch, yaw);
        hasData = true;
        Invalidate();
    }

    public bool SetCurrentAsZero()
    {
        if (!hasData) return false;
        zeroOrientation = sensorOrientation;
        zeroRoll = roll;
        zeroPitch = pitch;
        zeroYaw = yaw;
        hasZero = true;
        Invalidate();
        return true;
    }

    public void AdvanceHandleCounterClockwise()
    {
        handleAngle = (handleAngle + HandleStepDegrees) % 360.0f;
        Invalidate();
    }

    public void ClearZero()
    {
        hasZero = false;
        hasData = false;
        zeroOrientation = Matrix3.Identity;
        Invalidate();
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        base.OnPaint(e);
        e.Graphics.SmoothingMode = SmoothingMode.AntiAlias;
        DrawReference(e.Graphics);

        float shownRoll = hasZero ? Wrap180(roll - zeroRoll) : roll;
        float shownPitch = hasZero ? Wrap180(pitch - zeroPitch) : pitch;
        float shownYaw = hasZero ? Wrap180(yaw - zeroYaw) : yaw;
        Matrix3 displayOrientation = CreateOrientation(shownRoll, shownPitch, shownYaw);
        Point3[] rotated = model.Select(point => displayOrientation.Transform(point)).ToArray();
        DrawModel(e.Graphics, rotated, faces);
        DrawPhotoSurface(e.Graphics, displayOrientation);

        float handleRadians = DegreesToRadians(-handleAngle);
        Point3[] rotatedHandle = handleModel
            .Select(point => RotateHandleAroundRightAxis(point, handleRadians))
            .Select(point => displayOrientation.Transform(point))
            .ToArray();
        DrawModel(e.Graphics, rotatedHandle, handleFaces);
        DrawAxes(e.Graphics, displayOrientation);

        using Font angleFont = new("Consolas", 8F);
        using Brush angleBrush = new SolidBrush(Color.DimGray);
        string prefix = hasZero ? "ZERO  " : "RAW   ";
        e.Graphics.DrawString(string.Format(CultureInfo.InvariantCulture,
            "{0}R {1,6:0.0}  P {2,6:0.0}  Y {3,6:0.0}", prefix, shownRoll, shownPitch, shownYaw),
            angleFont, angleBrush, 5, Height - 19);
    }

    private void DrawModel(Graphics graphics, Point3[] rotated, Face[] modelFaces)
    {
        ProjectedPoint[] projected = rotated.Select(Project).ToArray();
        Face[] orderedFaces = modelFaces
            .OrderByDescending(face => face.Indices.Average(index => projected[index].Depth))
            .ToArray();

        using Pen edge = new(Color.FromArgb(45, 53, 62), 1.8f);
        using Font faceFont = new("맑은 고딕", 8.5F, FontStyle.Bold);
        using Brush textBrush = new SolidBrush(Color.FromArgb(35, 39, 45));
        using StringFormat centered = new() { Alignment = StringAlignment.Center, LineAlignment = StringAlignment.Center };
        foreach (Face face in orderedFaces)
        {
            PointF[] polygon = face.Indices.Select(index => projected[index].Screen).ToArray();
            using Brush fill = new SolidBrush(Color.FromArgb(225, face.Color));
            graphics.FillPolygon(fill, polygon);
            graphics.DrawPolygon(edge, polygon);
            if (face.Label.Length == 0) continue;
            float x = polygon.Average(point => point.X);
            float y = polygon.Average(point => point.Y);
            graphics.DrawString(face.Label, faceFont, textBrush, new PointF(x, y), centered);
        }
    }

    private static Point3 RotateHandleAroundRightAxis(Point3 point, float radians)
    {
        const float pivotY = -0.02f * TopScale;
        const float pivotZ = 0.03f * FrontScale;
        Point3 relative = new(point.X, point.Y - pivotY, point.Z - pivotZ);
        Point3 rotated = Matrix3.RotateX(radians).Transform(relative);
        return new Point3(rotated.X, rotated.Y + pivotY, rotated.Z + pivotZ);
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            reelTopImage?.Dispose();
            reelBottomImage?.Dispose();
        }
        base.Dispose(disposing);
    }

    private static Image? LoadEmbeddedImage(string resourceName)
    {
        using Stream? stream = typeof(ImuOrientationView).Assembly.GetManifestResourceStream(resourceName);
        if (stream == null) return null;
        using Image source = Image.FromStream(stream);
        // 배경 격자가 이미지에 굳어 들어간 자산은 표시하지 않고 3D 모델로 대체한다.
        if (!Image.IsAlphaPixelFormat(source.PixelFormat)) return null;
        return new Bitmap(source);
    }

    private void DrawPhotoSurface(Graphics graphics, Matrix3 orientation)
    {
        Point3 topNormal = orientation.Transform(new Point3(0, 1, 0));
        float facing = Dot(topNormal, Normalize(camera));
        Image? image = facing >= 0 ? reelTopImage : reelBottomImage;
        if (image == null) return;

        float opacity = Math.Clamp((MathF.Abs(facing) - 0.03f) / 0.14f, 0.0f, 1.0f);
        if (opacity <= 0.01f) return;

        const float minX = -0.92f * RightScale;
        const float maxX = 1.94f * RightScale;
        const float minZ = -0.82f * FrontScale;
        const float maxZ = 0.82f * FrontScale;
        float surfaceY = (facing >= 0 ? 0.69f : -0.69f) * TopScale;

        PointF[] destination;
        RectangleF sourceRectangle;
        if (facing >= 0)
        {
            const float bodyFraction = 0.62f;
            float photoMaxX = minX + (maxX - minX) * bodyFraction;
            destination = new[]
            {
                Project(orientation.Transform(new Point3(minX, surfaceY, minZ))).Screen,
                Project(orientation.Transform(new Point3(photoMaxX, surfaceY, minZ))).Screen,
                Project(orientation.Transform(new Point3(minX, surfaceY, maxZ))).Screen
            };
            sourceRectangle = new RectangleF(0, 0, image.Width * bodyFraction, image.Height);
        }
        else
        {
            const float handleFraction = 0.34f;
            float photoMaxX = maxX - (maxX - minX) * handleFraction;
            destination = new[]
            {
                Project(orientation.Transform(new Point3(photoMaxX, surfaceY, minZ))).Screen,
                Project(orientation.Transform(new Point3(minX, surfaceY, minZ))).Screen,
                Project(orientation.Transform(new Point3(photoMaxX, surfaceY, maxZ))).Screen
            };
            sourceRectangle = new RectangleF(image.Width * handleFraction, 0,
                image.Width * (1.0f - handleFraction), image.Height);
        }

        ColorMatrix alpha = new(new[]
        {
            new[] { 1.0f, 0, 0, 0, 0 },
            new[] { 0, 1.0f, 0, 0, 0 },
            new[] { 0, 0, 1.0f, 0, 0 },
            new[] { 0, 0, 0, opacity, 0 },
            new[] { 0, 0, 0, 0, 1.0f }
        });
        using ImageAttributes attributes = new();
        attributes.SetColorMatrix(alpha);
        graphics.DrawImage(image, destination, sourceRectangle, GraphicsUnit.Pixel, attributes);
    }

    private static void BuildReelModel(out Point3[] points, out Face[] modelFaces,
        out Point3[] handlePoints, out Face[] handleModelFaces)
    {
        List<Point3> vertices = new();
        List<Face> surfaces = new();
        List<Point3> handleVertices = new();
        List<Face> handleSurfaces = new();

        Color bodyFront = Color.FromArgb(48, 52, 61);
        Color bodyBack = Color.FromArgb(38, 42, 49);
        Color bodySide = Color.FromArgb(61, 66, 76);
        Color black = Color.FromArgb(30, 33, 39);
        Color blackHighlight = Color.FromArgb(57, 62, 70);
        Color silver = Color.FromArgb(151, 155, 160);
        Color screw = Color.FromArgb(205, 210, 215);

        PointF[] bodyProfile =
        {
            new(-0.72f, -0.72f), new(0.43f, -0.72f), new(0.72f, -0.48f), new(0.78f, 0.40f),
            new(0.54f, 0.66f), new(-0.55f, 0.66f), new(-0.78f, 0.43f), new(-0.80f, -0.48f)
        };
        AddExtrusion(vertices, surfaces, bodyProfile, -0.49f, 0.49f, bodyFront, bodyBack, bodySide);

        // RIGHT 방향으로 놓인 스풀, 회전축과 핸들
        AddCylinderX(vertices, surfaces, 0.48f, 1.18f, 0.03f, 0.02f, 0.47f, 14, blackHighlight, black);
        AddCylinderX(vertices, surfaces, 1.14f, 1.43f, 0.03f, 0.02f, 0.16f, 12, silver, blackHighlight);
        AddCrankArmX(handleVertices, handleSurfaces, 1.39f, 1.52f, 0.03f, 0.02f, -0.60f, -0.44f, 0.10f, black);
        AddCylinderX(handleVertices, handleSurfaces, 1.46f, 1.92f, -0.60f, -0.44f, 0.22f, 12, blackHighlight, black);

        // 뒷면 사진의 은색 고정 브래킷과 네 개의 체결부
        AddBox(vertices, surfaces, -0.22f, 0.22f, -0.62f, 0.58f, 0.50f, 0.57f, silver, silver, Color.FromArgb(105, 109, 115), "BACK");
        foreach (float screwX in new[] { -0.14f, 0.14f })
        foreach (float screwY in new[] { -0.43f, 0.39f })
            AddCylinderZ(vertices, surfaces, 0.57f, 0.62f, screwX, screwY, 0.055f, 8, screw, Color.FromArgb(128, 134, 140));

        // 장착 기준 자세에서는 금색 조작면이 FRONT가 아니라 TOP을 향한다.
        // RIGHT(X) 축은 그대로 두고 모델만 X축 기준으로 90도 회전한다.
        points = vertices.Select(point => new Point3(
            point.X * RightScale,
            -point.Z * TopScale,
            point.Y * FrontScale)).ToArray();
        modelFaces = surfaces.ToArray();
        handlePoints = handleVertices.Select(point => new Point3(
            point.X * RightScale,
            -point.Z * TopScale,
            point.Y * FrontScale)).ToArray();
        handleModelFaces = handleSurfaces.ToArray();
    }

    private static void AddExtrusion(List<Point3> points, List<Face> faces, PointF[] profile,
        float frontZ, float backZ, Color front, Color back, Color side, string frontLabel = "")
    {
        int start = points.Count;
        points.AddRange(profile.Select(point => new Point3(point.X, point.Y, frontZ)));
        points.AddRange(profile.Select(point => new Point3(point.X, point.Y, backZ)));
        int count = profile.Length;
        faces.Add(new Face(Enumerable.Range(start, count).ToArray(), front, frontLabel));
        faces.Add(new Face(Enumerable.Range(start + count, count).Reverse().ToArray(), back, ""));
        for (int i = 0; i < count; i++)
        {
            int next = (i + 1) % count;
            faces.Add(new Face(new[] { start + i, start + next, start + count + next, start + count + i }, side, ""));
        }
    }

    private static void AddBox(List<Point3> points, List<Face> faces,
        float minX, float maxX, float minY, float maxY, float minZ, float maxZ,
        Color front, Color back, Color side, string backLabel = "")
    {
        int start = points.Count;
        points.AddRange(new[]
        {
            new Point3(minX, minY, minZ), new Point3(maxX, minY, minZ),
            new Point3(maxX, maxY, minZ), new Point3(minX, maxY, minZ),
            new Point3(minX, minY, maxZ), new Point3(maxX, minY, maxZ),
            new Point3(maxX, maxY, maxZ), new Point3(minX, maxY, maxZ)
        });
        faces.Add(new Face(new[] { start, start + 1, start + 2, start + 3 }, front, ""));
        faces.Add(new Face(new[] { start + 7, start + 6, start + 5, start + 4 }, back, backLabel));
        faces.Add(new Face(new[] { start, start + 4, start + 5, start + 1 }, side, ""));
        faces.Add(new Face(new[] { start + 1, start + 5, start + 6, start + 2 }, side, ""));
        faces.Add(new Face(new[] { start + 2, start + 6, start + 7, start + 3 }, side, ""));
        faces.Add(new Face(new[] { start + 3, start + 7, start + 4, start }, side, ""));
    }

    private static void AddCylinderX(List<Point3> points, List<Face> faces, float minX, float maxX,
        float centerY, float centerZ, float radius, int segments, Color cap, Color side)
    {
        int start = points.Count;
        for (int i = 0; i < segments; i++)
        {
            float angle = MathF.Tau * i / segments;
            float y = centerY + MathF.Cos(angle) * radius;
            float z = centerZ + MathF.Sin(angle) * radius;
            points.Add(new Point3(minX, y, z));
            points.Add(new Point3(maxX, y, z));
        }
        faces.Add(new Face(Enumerable.Range(0, segments).Select(i => start + i * 2).Reverse().ToArray(), cap, ""));
        faces.Add(new Face(Enumerable.Range(0, segments).Select(i => start + i * 2 + 1).ToArray(), cap, ""));
        for (int i = 0; i < segments; i++)
        {
            int next = (i + 1) % segments;
            faces.Add(new Face(new[] { start + i * 2, start + next * 2, start + next * 2 + 1, start + i * 2 + 1 }, side, ""));
        }
    }

    private static void AddCylinderZ(List<Point3> points, List<Face> faces, float minZ, float maxZ,
        float centerX, float centerY, float radius, int segments, Color cap, Color side)
    {
        int start = points.Count;
        for (int i = 0; i < segments; i++)
        {
            float angle = MathF.Tau * i / segments;
            float x = centerX + MathF.Cos(angle) * radius;
            float y = centerY + MathF.Sin(angle) * radius;
            points.Add(new Point3(x, y, minZ));
            points.Add(new Point3(x, y, maxZ));
        }
        faces.Add(new Face(Enumerable.Range(0, segments).Select(i => start + i * 2).Reverse().ToArray(), cap, ""));
        faces.Add(new Face(Enumerable.Range(0, segments).Select(i => start + i * 2 + 1).ToArray(), cap, ""));
        for (int i = 0; i < segments; i++)
        {
            int next = (i + 1) % segments;
            faces.Add(new Face(new[] { start + i * 2, start + next * 2, start + next * 2 + 1, start + i * 2 + 1 }, side, ""));
        }
    }

    private static void AddCrankArmX(List<Point3> points, List<Face> faces, float minX, float maxX,
        float startY, float startZ, float endY, float endZ, float width, Color color)
    {
        float dy = endY - startY;
        float dz = endZ - startZ;
        float length = MathF.Sqrt(dy * dy + dz * dz);
        float py = -dz / length * width;
        float pz = dy / length * width;
        PointF[] profile =
        {
            new(startY + py, startZ + pz), new(endY + py, endZ + pz),
            new(endY - py, endZ - pz), new(startY - py, startZ - pz)
        };
        int start = points.Count;
        points.AddRange(profile.Select(point => new Point3(minX, point.X, point.Y)));
        points.AddRange(profile.Select(point => new Point3(maxX, point.X, point.Y)));
        faces.Add(new Face(new[] { start, start + 1, start + 2, start + 3 }, color, ""));
        faces.Add(new Face(new[] { start + 7, start + 6, start + 5, start + 4 }, color, ""));
        for (int i = 0; i < 4; i++)
        {
            int next = (i + 1) % 4;
            faces.Add(new Face(new[] { start + i, start + next, start + 4 + next, start + 4 + i }, color, ""));
        }
    }

    private void DrawReference(Graphics graphics)
    {
        int baseline = Height - 29;
        using Pen line = new(Color.FromArgb(218, 221, 225), 1F);
        graphics.DrawLine(line, 18, baseline, Width - 18, baseline);
        using Brush shadow = new SolidBrush(Color.FromArgb(22, 35, 40, 48));
        graphics.FillEllipse(shadow, Width * 0.22f, baseline - 8, Width * 0.58f, 15);
    }

    private void DrawAxes(Graphics graphics, Matrix3 orientation)
    {
        ProjectedPoint center = Project(orientation.Transform(new Point3(0, 0, 0)));
        DrawAxis(graphics, center.Screen, Project(orientation.Transform(new Point3(0, 0, -1.45f))).Screen,
            Color.FromArgb(210, 45, 45), "FRONT");
        DrawAxis(graphics, center.Screen, Project(orientation.Transform(new Point3(0, 1.25f, 0))).Screen,
            Color.FromArgb(45, 105, 220), "TOP");
        DrawAxis(graphics, center.Screen, Project(orientation.Transform(new Point3(1.35f, 0, 0))).Screen,
            Color.FromArgb(35, 155, 75), "RIGHT");
    }

    private static void DrawAxis(Graphics graphics, PointF start, PointF end, Color color, string label)
    {
        using Pen outline = new(Color.FromArgb(210, Color.White), 7.0f) { StartCap = LineCap.Round, EndCap = LineCap.ArrowAnchor };
        using Pen arrow = new(color, 4.5f) { StartCap = LineCap.Round, EndCap = LineCap.ArrowAnchor };
        graphics.DrawLine(outline, start, end);
        graphics.DrawLine(arrow, start, end);
        using Font font = new("맑은 고딕", 7.5F, FontStyle.Bold);
        using Brush brush = new SolidBrush(color);
        float labelX = end.X + (end.X >= start.X ? 4 : -34);
        float labelY = end.Y + (end.Y >= start.Y ? 3 : -15);
        graphics.DrawString(label, font, brush, labelX, labelY);
    }

    private ProjectedPoint Project(Point3 point)
    {
        Point3 relative = new(point.X - camera.X, point.Y - camera.Y, point.Z - camera.Z);
        float viewX = Dot(relative, cameraRight);
        float viewY = Dot(relative, cameraUp);
        float depth = Math.Max(0.4f, Dot(relative, cameraForward));
        float focal = Math.Min(Width, Height) * 2.25f;
        float centerX = Width * 0.50f;
        float centerY = Height * 0.46f;
        return new ProjectedPoint(new PointF(centerX + focal * viewX / depth, centerY - focal * viewY / depth), depth);
    }

    private static Matrix3 CreateOrientation(float rollDegrees, float pitchDegrees, float yawDegrees)
    {
        float roll = DegreesToRadians(-rollDegrees);
        float pitch = DegreesToRadians(pitchDegrees);
        float yaw = DegreesToRadians(-yawDegrees);
        return Matrix3.RotateY(yaw) * Matrix3.RotateX(pitch) * Matrix3.RotateZ(roll);
    }

    private static float DegreesToRadians(float value) => value * MathF.PI / 180.0f;
    private static float Wrap180(float value)
    {
        while (value > 180.0f) value -= 360.0f;
        while (value <= -180.0f) value += 360.0f;
        return value;
    }

    private static float Dot(Point3 left, Point3 right) => left.X * right.X + left.Y * right.Y + left.Z * right.Z;
    private static Point3 Cross(Point3 left, Point3 right) => new(
        left.Y * right.Z - left.Z * right.Y,
        left.Z * right.X - left.X * right.Z,
        left.X * right.Y - left.Y * right.X);
    private static Point3 Normalize(Point3 point)
    {
        float length = MathF.Sqrt(Dot(point, point));
        return new Point3(point.X / length, point.Y / length, point.Z / length);
    }

    private readonly record struct Point3(float X, float Y, float Z);
    private readonly record struct ProjectedPoint(PointF Screen, float Depth);
    private sealed record Face(int[] Indices, Color Color, string Label);

    private readonly record struct Matrix3(
        float M11, float M12, float M13,
        float M21, float M22, float M23,
        float M31, float M32, float M33)
    {
        public static Matrix3 Identity => new(1, 0, 0, 0, 1, 0, 0, 0, 1);

        public Point3 Transform(Point3 point) => new(
            M11 * point.X + M12 * point.Y + M13 * point.Z,
            M21 * point.X + M22 * point.Y + M23 * point.Z,
            M31 * point.X + M32 * point.Y + M33 * point.Z);

        public static Matrix3 RotateX(float angle)
        {
            float c = MathF.Cos(angle), s = MathF.Sin(angle);
            return new Matrix3(1, 0, 0, 0, c, -s, 0, s, c);
        }

        public static Matrix3 RotateY(float angle)
        {
            float c = MathF.Cos(angle), s = MathF.Sin(angle);
            return new Matrix3(c, 0, s, 0, 1, 0, -s, 0, c);
        }

        public static Matrix3 RotateZ(float angle)
        {
            float c = MathF.Cos(angle), s = MathF.Sin(angle);
            return new Matrix3(c, -s, 0, s, c, 0, 0, 0, 1);
        }

        public static Matrix3 Transpose(Matrix3 value) => new(
            value.M11, value.M21, value.M31,
            value.M12, value.M22, value.M32,
            value.M13, value.M23, value.M33);

        public static Matrix3 operator *(Matrix3 left, Matrix3 right) => new(
            left.M11 * right.M11 + left.M12 * right.M21 + left.M13 * right.M31,
            left.M11 * right.M12 + left.M12 * right.M22 + left.M13 * right.M32,
            left.M11 * right.M13 + left.M12 * right.M23 + left.M13 * right.M33,
            left.M21 * right.M11 + left.M22 * right.M21 + left.M23 * right.M31,
            left.M21 * right.M12 + left.M22 * right.M22 + left.M23 * right.M32,
            left.M21 * right.M13 + left.M22 * right.M23 + left.M23 * right.M33,
            left.M31 * right.M11 + left.M32 * right.M21 + left.M33 * right.M31,
            left.M31 * right.M12 + left.M32 * right.M22 + left.M33 * right.M32,
            left.M31 * right.M13 + left.M32 * right.M23 + left.M33 * right.M33);
    }
}
