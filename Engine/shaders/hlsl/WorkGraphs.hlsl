// Seems Slang Shading Language supports Work Graphs but no documentation is shown yet.
[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]

[NumThreads(1, 1, 1)]
void BroadcastNode()
{
    
}
