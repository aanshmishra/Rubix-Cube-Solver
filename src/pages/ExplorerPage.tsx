import { useState, useCallback } from 'react';
import { useCubeStore } from '@/store/useCubeStore';
import type { FaceColor } from '@/types';
import {
  applyMoveToFacelets,
  getPrunedMoves,
  hashFacelets,
  type MoveName,
} from '@/lib/cubeEngine';
import { Button } from '@/components/ui/button';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Label } from '@/components/ui/label';
import { Switch } from '@/components/ui/switch';
import { Badge } from '@/components/ui/badge';
import { Separator } from '@/components/ui/separator';
import {
  GitBranch,
  Network,
  Play,
  ChevronRight,
  ChevronDown,
  RotateCcw,
  AlertCircle,
  Layers,
} from 'lucide-react';

interface TreeNode {
  id: string;
  move: string;
  moveNotation: string;
  depth: number;
  stateHash: number;
  expanded: boolean;
  children: string[];
  path: string[];
  facelets: FaceColor[];
  isGraphMerge?: boolean; // If true, this node merges with an existing state
  canonicalId?: string;
}

export default function ExplorerPage() {
  const { facelets } = useCubeStore();
  const [graphMode, setGraphMode] = useState(false);
  const [nodes, setNodes] = useState<Map<string, TreeNode>>(new Map());
  const [rootId, setRootId] = useState<string | null>(null);
  const [error, setError] = useState('');

  const initializeTree = useCallback(async () => {
    setError('');
    try {
      const rootNode: TreeNode = {
        id: 'root',
        move: 'root',
        moveNotation: 'Start',
        depth: 0,
        stateHash: hashFacelets(facelets),
        expanded: false,
        children: [],
        path: [],
        facelets: [...facelets],
      };

      const newNodes = new Map<string, TreeNode>();
      newNodes.set('root', rootNode);
      setNodes(newNodes);
      setRootId('root');
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to initialize');
    }
  }, [facelets]);

  const expandNode = useCallback(async (nodeId: string) => {
    const node = nodes.get(nodeId);
    if (!node || node.depth >= 5 || node.expanded) return;

    try {
      const lastMove = node.path.length > 0 ? node.path[node.path.length - 1] : undefined;
      const validMoves = getPrunedMoves(lastMove);

      const newNodes = new Map(nodes);
      const children: string[] = [];

      for (const move of validMoves) {
        const childId = `${nodeId}-${move}`;
        const childPath = [...node.path, move as MoveName];
        const childFacelets = applyMoveToFacelets(node.facelets, move);
        const stateHash = hashFacelets(childFacelets);

        const childNode: TreeNode = {
          id: childId,
          move,
          moveNotation: move,
          depth: node.depth + 1,
          stateHash,
          expanded: false,
          children: [],
          path: childPath,
          facelets: childFacelets,
        };

        if (graphMode) {
          for (const [existingId, existingNode] of newNodes) {
            if (existingNode.stateHash === childNode.stateHash && existingId !== childId) {
              childNode.isGraphMerge = true;
              childNode.canonicalId = existingId;
              break;
            }
          }
        }

        newNodes.set(childId, childNode);
        children.push(childId);
      }

      const updatedNode = newNodes.get(nodeId);
      if (updatedNode) {
        updatedNode.expanded = true;
        updatedNode.children = children;
      }

      setNodes(newNodes);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to expand node');
    }
  }, [nodes, graphMode]);

  const collapseNode = useCallback((nodeId: string) => {
    const newNodes = new Map(nodes);
    const node = newNodes.get(nodeId);
    if (node) {
      node.expanded = false;
      // Recursively collapse children
      const collapseChildren = (parentId: string) => {
        const parent = newNodes.get(parentId);
        if (!parent) return;
        for (const childId of parent.children) {
          const child = newNodes.get(childId);
          if (child) {
            child.expanded = false;
            collapseChildren(childId);
          }
        }
      };
      collapseChildren(nodeId);
    }
    setNodes(newNodes);
  }, [nodes]);

  const toggleNode = useCallback((nodeId: string) => {
    const node = nodes.get(nodeId);
    if (!node) return;
    if (node.expanded) {
      collapseNode(nodeId);
    } else {
      expandNode(nodeId);
    }
  }, [nodes, expandNode, collapseNode]);

  const renderTree = (nodeId: string, indent: number = 0) => {
    const node = nodes.get(nodeId);
    if (!node) return null;

    const isExpandable = node.depth < 5;

    return (
      <div key={nodeId}>
        <div
          className="flex items-center gap-2 py-1 px-2 rounded-md hover:bg-muted cursor-pointer transition-colors"
          style={{ paddingLeft: `${indent * 16 + 8}px` }}
          onClick={() => isExpandable && toggleNode(nodeId)}
        >
          {isExpandable ? (
            node.expanded ? (
              <ChevronDown className="h-3.5 w-3.5 text-muted-foreground" />
            ) : (
              <ChevronRight className="h-3.5 w-3.5 text-muted-foreground" />
            )
          ) : (
            <div className="w-3.5" />
          )}

          <span className="text-xs font-mono w-6 text-muted-foreground">{node.depth}</span>

          {nodeId === 'root' ? (
            <Badge variant="outline" className="text-xs">Start</Badge>
          ) : (
            <>
              <span className="text-sm font-mono font-medium">{node.moveNotation}</span>
              {node.isGraphMerge && (
                <Badge variant="secondary" className="text-xs">
                  <Network className="h-3 w-3 mr-1" />
                  merge
                </Badge>
              )}
            </>
          )}

          <span className="text-xs text-muted-foreground ml-auto">
            #{node.stateHash.toString(16).substr(0, 6)}
          </span>
        </div>

        {node.expanded && node.children.map((childId) => renderTree(childId, indent + 1))}
      </div>
    );
  };

  return (
    <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
      {/* Left: Controls */}
      <div className="space-y-6">
        <Card>
          <CardHeader className="pb-3">
            <CardTitle className="text-lg flex items-center gap-2">
              <GitBranch className="h-4 w-4" />
              Move Tree Explorer
            </CardTitle>
          </CardHeader>
          <CardContent className="space-y-4">
            <p className="text-sm text-muted-foreground">
              Explore all possible move sequences up to depth {5}. Click nodes to expand.
            </p>

            <div className="flex items-center justify-between">
              <Label htmlFor="graph-mode" className="flex items-center gap-2 cursor-pointer">
                <Network className="h-4 w-4" />
                Graph Mode
              </Label>
              <Switch
                id="graph-mode"
                checked={graphMode}
                onCheckedChange={setGraphMode}
              />
            </div>

            <p className="text-xs text-muted-foreground">
              {graphMode
                ? 'Merges states reached by different paths into single nodes'
                : 'Pure tree: each path is a separate branch'}
            </p>

            <Separator />

            <div className="flex items-center gap-2">
              <Layers className="h-4 w-4 text-muted-foreground" />
              <span className="text-sm">Depth cap:</span>
              <Badge variant="outline">5</Badge>
            </div>

            <div className="flex items-center gap-2 text-xs text-muted-foreground">
              <AlertCircle className="h-3.5 w-3.5" />
              Same-face repeats and opposite-face commutations are pruned
            </div>

            <Button
              className="w-full"
              onClick={initializeTree}
            >
              <Play className="h-4 w-4 mr-2" />
              Initialize from Current Cube
            </Button>

            {rootId && (
              <Button
                variant="outline"
                className="w-full"
                onClick={() => {
                  setNodes(new Map());
                  setRootId(null);
                }}
              >
                <RotateCcw className="h-4 w-4 mr-2" />
                Reset
              </Button>
            )}

            {error && (
              <div className="flex items-center gap-2 text-destructive text-sm">
                <AlertCircle className="h-4 w-4" />
                {error}
              </div>
            )}
          </CardContent>
        </Card>

        {/* Stats */}
        {rootId && (
          <Card>
            <CardHeader className="pb-3">
              <CardTitle className="text-lg">Statistics</CardTitle>
            </CardHeader>
            <CardContent className="space-y-2">
              <div className="flex justify-between text-sm">
                <span className="text-muted-foreground">Total nodes:</span>
                <span className="font-mono">{nodes.size}</span>
              </div>
              <div className="flex justify-between text-sm">
                <span className="text-muted-foreground">Expanded:</span>
                <span className="font-mono">
                  {Array.from(nodes.values()).filter((n) => n.expanded).length}
                </span>
              </div>
              <div className="flex justify-between text-sm">
                <span className="text-muted-foreground">Max depth:</span>
                <span className="font-mono">
                  {Math.max(...Array.from(nodes.values()).map((n) => n.depth))}
                </span>
              </div>
              {graphMode && (
                <div className="flex justify-between text-sm">
                  <span className="text-muted-foreground">Merges:</span>
                  <span className="font-mono">
                    {Array.from(nodes.values()).filter((n) => n.isGraphMerge).length}
                  </span>
                </div>
              )}
            </CardContent>
          </Card>
        )}
      </div>

      {/* Right: Tree visualization */}
      <div className="lg:col-span-2">
        <Card className="h-full">
          <CardHeader className="pb-3">
            <CardTitle className="text-lg">
              {graphMode ? 'State Graph' : 'Move Tree'}
            </CardTitle>
          </CardHeader>
          <CardContent>
            {rootId ? (
              <div className="overflow-auto max-h-[600px] border rounded-lg p-2">
                {renderTree(rootId)}
              </div>
            ) : (
              <div className="text-center py-12 text-muted-foreground">
                <GitBranch className="h-12 w-12 mx-auto mb-4" />
                <p>Click "Initialize" to start exploring</p>
              </div>
            )}
          </CardContent>
        </Card>
      </div>
    </div>
  );
}
