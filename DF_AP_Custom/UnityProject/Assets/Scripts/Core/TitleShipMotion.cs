using UnityEngine;

namespace FishingGame
{
    /// <summary>
    /// Lightweight replacement for the original SWS splineMove dependency.
    /// Waypoints and speeds are copied from the serialized Title Scene evidence.
    /// </summary>
    public sealed class TitleShipMotion : MonoBehaviour
    {
        [SerializeField] private Vector3[] waypoints;
        [SerializeField, Min(0.01f)] private float speed = 10f;
        [SerializeField] private bool loop;
        [SerializeField] private bool orientToPath = true;

        private int targetIndex;

        public void Configure(Vector3[] points, float movementSpeed, bool shouldLoop)
        {
            waypoints = points;
            speed = movementSpeed;
            loop = shouldLoop;
        }

        private void Start()
        {
            if (waypoints == null || waypoints.Length == 0)
            {
                enabled = false;
                return;
            }

            transform.position = waypoints[0];
            targetIndex = Mathf.Min(1, waypoints.Length - 1);
        }

        private void Update()
        {
            if (waypoints == null || waypoints.Length < 2)
            {
                return;
            }

            Vector3 target = waypoints[targetIndex];
            Vector3 delta = target - transform.position;
            if (orientToPath && delta.sqrMagnitude > 0.0001f)
            {
                Vector3 horizontal = Vector3.ProjectOnPlane(delta, Vector3.up);
                if (horizontal.sqrMagnitude > 0.0001f)
                {
                    transform.rotation = Quaternion.Slerp(transform.rotation, Quaternion.LookRotation(horizontal), Time.deltaTime * 2f);
                }
            }

            transform.position = Vector3.MoveTowards(transform.position, target, speed * Time.deltaTime);
            if ((transform.position - target).sqrMagnitude > 0.01f)
            {
                return;
            }

            targetIndex++;
            if (targetIndex < waypoints.Length)
            {
                return;
            }

            if (loop)
            {
                targetIndex = 0;
            }
            else
            {
                enabled = false;
            }
        }
    }
}
