using System.Collections.Generic;
using System.IO;
using UnityEngine;

public class PathGraphDrawer : MonoBehaviour
{
    public GameObject linePrefab;
    public string pathFile;

    // Start is called before the first frame update
    void Start()
    {
        var content = File.ReadAllLines(pathFile);
        int lineCounter = 0;
        if (content.Length == 0 || content[lineCounter] != "PATHGRAPH")
        {
            return;
        }
        lineCounter++;
        var size = content[lineCounter].Split(' ');
        if (size.Length != 2)
        {
            return;
        }
        var vertexSize = int.Parse(size[0]);
        var linkSize = int.Parse(size[1]);
        lineCounter++;
        List<Vector3> positions = new List<Vector3>();
        for (var i = 0; i < vertexSize; i++)
        {
            var position = content[lineCounter].Split(' ');
            positions.Add(new Vector3(float.Parse(position[0]), float.Parse(position[1]), float.Parse(position[2])));

            lineCounter++;
        }
        for (var i = 0; i < linkSize; i++)
        {
            var link = content[lineCounter].Split(' ');
            var start = int.Parse(link[0]);
            var end = int.Parse(link[1]);

            var line = Instantiate(linePrefab).GetComponent<LineRenderer>();
            line.SetPositions(new Vector3[] { positions[start], positions[end] });

            lineCounter++;
        }
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
