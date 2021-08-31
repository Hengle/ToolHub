using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
using Unity.Mathematics;
using static Unity.Mathematics.math;
[RequireComponent(typeof(Camera))]
public class CustomRenderTarget : MonoBehaviour
{
    private static CustomRenderTarget current = null;
    private CommandBuffer beforePostBuffer;
    private CommandBuffer afterPostBuffer;
    private CommandBuffer afterOpaqueBuffer;
    private Camera cam;
    private Mesh screenMesh;
    private CommandBuffer GetBuffer(CameraEvent evt)
    {
        var bf = new CommandBuffer();
        cam.AddCommandBuffer(evt, bf);
        return bf;
    }
    public CommandBuffer BeforePostBuffer
    {
        get
        {
            if (beforePostBuffer != null) return beforePostBuffer;
            beforePostBuffer = GetBuffer(CameraEvent.BeforeImageEffects);
            return beforePostBuffer;
        }
    }
    public CommandBuffer AfterPostBuffer
    {
        get
        {
            if (afterPostBuffer != null) return afterPostBuffer;
            afterPostBuffer = GetBuffer(CameraEvent.AfterImageEffects);
            return afterPostBuffer;
        }
    }
    public CommandBuffer AfterOpaqueBuffer
    {
        get
        {
            if (afterOpaqueBuffer != null) return afterOpaqueBuffer;
            afterOpaqueBuffer = GetBuffer(CameraEvent.AfterImageEffectsOpaque);
            return afterOpaqueBuffer;
        }
    }
    private void Awake()
    {
        if (current)
        {
            Debug.LogError("CustomRT Must be singleton");
        }
        cam = GetComponent<Camera>();
        screenMesh = new Mesh();
        screenMesh.vertices = new Vector3[4]
        {
            new Vector3(0,0,1),
            new Vector3(0,1,1),
            new Vector3(1,0,1),
            new Vector3(1,1,1)
        };
        screenMesh.subMeshCount = 1;
        screenMesh.SetIndices(new int[] {
        0,1,2,
        0,2,3
        },
        MeshTopology.Triangles,
        0);
    }
    public static void BlitPart(
        Texture source,
        RenderTexture dest,
        float2 sourceStartUV,
        float2 sourceEndUV,
        float2 desetStartUV,
        float2 destEndUV)
    {

    }
}
