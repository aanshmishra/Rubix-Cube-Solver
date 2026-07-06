import { useState, useEffect, useCallback } from 'react';
import { useCubeStore } from '@/store/useCubeStore';
import { loadWasmModule } from '@/wasm/wasmLoader';
import { getMethodName, type SolutionResult } from '@/types';
import { saveSolveEntry, createSolveEntry } from '@/storage/history';
import { Button } from '@/components/ui/button';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Label } from '@/components/ui/label';
import { Switch } from '@/components/ui/switch';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';
import { Separator } from '@/components/ui/separator';
import { Badge } from '@/components/ui/badge';
import {
  RotateCcw,
  Shuffle,
  Play,
  Pause,
  Check,
  AlertCircle,
  Settings,
  Cuboid,
  Network,
  Route,
  Search,
} from 'lucide-react';
import CubeNet from '@/components/CubeNet';
import MovePlayer from '@/components/MovePlayer';

export default function SolverPage() {
  const {
    facelets,
    scrambleNotation,
    orientation,
    methodConfig,
    solution,
    setSolution,
    isSolving,
    setIsSolving,
    generateNewScramble,
    setOrientation,
    toggleF2L,
    toggleOLL,
    togglePLL,
    resetSolved,
    applyScramble,
  } = useCubeStore();

  const [wasmReady, setWasmReady] = useState(false);
  const [error, setError] = useState('');
  const [activeStage, setActiveStage] = useState<'cross' | 'f2l' | 'oll' | 'pll'>('cross');
  const [showPlayer, setShowPlayer] = useState(false);
  const [currentMoveIndex, setCurrentMoveIndex] = useState(0);

  useEffect(() => {
    loadWasmModule().then(() => setWasmReady(true)).catch(() => setWasmReady(false));
  }, []);

  const handleSolve = useCallback(async () => {
    setIsSolving(true);
    setError('');
    setSolution(null);
    setShowPlayer(false);
    setCurrentMoveIndex(0);

    try {
      const wasm = await loadWasmModule();
      const faceletStr = facelets.join('');

      // Validate
      const validation = wasm.validateFacelets(faceletStr);
      if (!validation.valid) {
        setError(validation.errorMessage);
        setIsSolving(false);
        return;
      }

      // Convert to state
      const state = wasm.cubeStateFromFacelets(faceletStr, orientation);
      const stateValidation = wasm.validateCubeState(state);
      if (!stateValidation.valid) {
        setError(stateValidation.errorMessage);
        setIsSolving(false);
        return;
      }

      // Build method config for WASM
      const wasmConfig = {
        f2lAdvanced: methodConfig.f2lAdvanced,
        ollAdvanced: methodConfig.ollAdvanced,
        pllAdvanced: methodConfig.pllAdvanced,
      };

      // Solve
      const wasmResult = wasm.solveCube(state, wasmConfig);

      if (!wasmResult.success) {
        setError(wasmResult.errorMessage || 'Failed to solve cube');
        setIsSolving(false);
        return;
      }

      // Convert to JS format
      const result: SolutionResult = {
        crossMoves: wasmResult.crossMoves.map((m: number) => wasm.moveToString(m)),
        f2lMoves: wasmResult.f2lMoves.map((m: number) => wasm.moveToString(m)),
        ollMoves: wasmResult.ollMoves.map((m: number) => wasm.moveToString(m)),
        pllMoves: wasmResult.pllMoves.map((m: number) => wasm.moveToString(m)),
        config: methodConfig,
        success: true,
        errorMessage: '',
        methodName: getMethodName(methodConfig),
        totalMoves: wasmResult.crossMoves.length + wasmResult.f2lMoves.length +
                     wasmResult.ollMoves.length + wasmResult.pllMoves.length,
      };

      setSolution(result);

      // Save to history
      const entry = createSolveEntry(
        faceletStr,
        scrambleNotation,
        result.methodName,
        methodConfig,
        result.crossMoves.length,
        result.f2lMoves.length,
        result.ollMoves.length,
        result.pllMoves.length,
        [
          ...result.crossMoves,
          ...result.f2lMoves,
          ...result.ollMoves,
          ...result.pllMoves,
        ].join(' ')
      );
      saveSolveEntry(entry);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Unknown error occurred');
    } finally {
      setIsSolving(false);
    }
  }, [facelets, orientation, methodConfig, scrambleNotation, setSolution, setIsSolving]);

  const allMoves = solution
    ? [...solution.crossMoves, ...solution.f2lMoves, ...solution.ollMoves, ...solution.pllMoves]
    : [];

  const stageMoves = solution
    ? activeStage === 'cross'
      ? solution.crossMoves
      : activeStage === 'f2l'
      ? solution.f2lMoves
      : activeStage === 'oll'
      ? solution.ollMoves
      : solution.pllMoves
    : [];

  const stageColor = (stage: string) => {
    switch (stage) {
      case 'cross': return 'bg-blue-500';
      case 'f2l': return 'bg-green-500';
      case 'oll': return 'bg-yellow-500';
      case 'pll': return 'bg-purple-500';
      default: return 'bg-gray-500';
    }
  };

  return (
    <div className="space-y-5">
      <section className="rounded-lg border bg-card/95 p-4 shadow-[0_1px_2px_hsl(var(--foreground)/0.05)] sm:p-5">
        <div className="flex flex-col gap-4 lg:flex-row lg:items-center lg:justify-between">
          <div>
            <div className="flex items-center gap-2">
              <div className="flex size-9 items-center justify-center rounded-md bg-primary text-primary-foreground">
                <Cuboid className="h-5 w-5" />
              </div>
              <div>
                <h2 className="text-xl font-semibold tracking-tight">Adaptive Solver</h2>
                <p className="text-sm text-muted-foreground">{getMethodName(methodConfig)}</p>
              </div>
            </div>
          </div>
          <div className="grid grid-cols-3 gap-2 text-sm">
            <div className="rounded-md border bg-background px-3 py-2">
              <div className="flex items-center gap-2 text-muted-foreground">
                <Search className="h-4 w-4 text-primary" />
                Cross
              </div>
              <div className="mt-1 font-medium">BFS</div>
            </div>
            <div className="rounded-md border bg-background px-3 py-2">
              <div className="flex items-center gap-2 text-muted-foreground">
                <Route className="h-4 w-4 text-accent" />
                F2L
              </div>
              <div className="mt-1 font-medium">{methodConfig.f2lAdvanced ? 'Search' : 'Beginner'}</div>
            </div>
            <div className="rounded-md border bg-background px-3 py-2">
              <div className="flex items-center gap-2 text-muted-foreground">
                <Network className="h-4 w-4 text-primary" />
                Explorer
              </div>
              <div className="mt-1 font-medium">Lazy BFS</div>
            </div>
          </div>
        </div>
      </section>

      <div className="grid grid-cols-1 gap-5 lg:grid-cols-[minmax(0,0.95fr)_minmax(0,1.05fr)]">
      {/* Left: Cube Input */}
      <div className="space-y-5">
        {/* Cube Net */}
        <Card>
          <CardHeader className="pb-3">
            <div className="flex items-center justify-between">
              <CardTitle className="text-lg">Cube Input</CardTitle>
              <div className="flex items-center gap-2">
                <Button variant="outline" size="sm" onClick={resetSolved}>
                  <RotateCcw className="h-3.5 w-3.5 mr-1" />
                  Solved
                </Button>
                <Button variant="outline" size="sm" onClick={generateNewScramble}>
                  <Shuffle className="h-3.5 w-3.5 mr-1" />
                  Scramble
                </Button>
              </div>
            </div>
          </CardHeader>
          <CardContent>
            <CubeNet />
          </CardContent>
        </Card>

        {/* Scramble Input */}
        <Card>
          <CardHeader className="pb-3">
            <CardTitle className="text-lg">Scramble</CardTitle>
          </CardHeader>
          <CardContent className="space-y-4">
            <div className="flex items-center gap-2">
              <input
                type="text"
                value={scrambleNotation}
                onChange={(e) => useCubeStore.getState().setScrambleNotation(e.target.value)}
                className="flex-1 px-3 py-2 rounded-md border bg-background text-sm font-mono"
                placeholder="R U R' U' ..."
              />
              <Button variant="outline" size="sm" onClick={generateNewScramble}>
                <Shuffle className="h-4 w-4" />
              </Button>
              <Button variant="secondary" size="sm" onClick={applyScramble}>
                Apply
              </Button>
            </div>
            <p className="text-xs text-muted-foreground">
              Enter scramble notation or click "Scramble" to generate a random one
            </p>
          </CardContent>
        </Card>

        {/* Orientation & Method Settings */}
        <Card>
          <CardHeader className="pb-3">
            <CardTitle className="text-lg flex items-center gap-2">
              <Settings className="h-4 w-4" />
              Settings
            </CardTitle>
          </CardHeader>
          <CardContent className="space-y-4">
            {/* Physical Orientation */}
            <div className="space-y-2">
              <Label>Physical Orientation</Label>
              <Select value={orientation} onValueChange={(v) => setOrientation(v as any)}>
                <SelectTrigger>
                  <SelectValue placeholder="Select orientation" />
                </SelectTrigger>
                <SelectContent>
                  <SelectItem value="WG">White Down, Green Front</SelectItem>
                  <SelectItem value="WB">White Down, Blue Front</SelectItem>
                  <SelectItem value="WR">White Down, Red Front</SelectItem>
                  <SelectItem value="WO">White Down, Orange Front</SelectItem>
                  <SelectItem value="YG">Yellow Down, Green Front</SelectItem>
                  <SelectItem value="YB">Yellow Down, Blue Front</SelectItem>
                  <SelectItem value="YR">Yellow Down, Red Front</SelectItem>
                  <SelectItem value="YO">Yellow Down, Orange Front</SelectItem>
                  <SelectItem value="GW">Green Down, White Front</SelectItem>
                  <SelectItem value="GY">Green Down, Yellow Front</SelectItem>
                  <SelectItem value="RG">Red Down, Green Front</SelectItem>
                  <SelectItem value="RB">Red Down, Blue Front</SelectItem>
                </SelectContent>
              </Select>
            </div>

            <Separator />

            {/* Method Toggles */}
            <div className="space-y-3">
              <Label>Method Configuration</Label>

              <div className="flex items-center justify-between">
                <div>
                  <p className="text-sm font-medium">F2L</p>
                  <p className="text-xs text-muted-foreground">
                    {methodConfig.f2lAdvanced ? 'Advanced (1-look pairs)' : 'Beginner (decomposed)'}
                  </p>
                </div>
                <Switch
                  checked={methodConfig.f2lAdvanced}
                  onCheckedChange={toggleF2L}
                />
              </div>

              <div className="flex items-center justify-between">
                <div>
                  <p className="text-sm font-medium">OLL</p>
                  <p className="text-xs text-muted-foreground">
                    {methodConfig.ollAdvanced ? 'Advanced (57 cases)' : 'Beginner (2-step)'}
                  </p>
                </div>
                <Switch
                  checked={methodConfig.ollAdvanced}
                  onCheckedChange={toggleOLL}
                />
              </div>

              <div className="flex items-center justify-between">
                <div>
                  <p className="text-sm font-medium">PLL</p>
                  <p className="text-xs text-muted-foreground">
                    {methodConfig.pllAdvanced ? 'Advanced (21 cases)' : 'Beginner (2-step)'}
                  </p>
                </div>
                <Switch
                  checked={methodConfig.pllAdvanced}
                  onCheckedChange={togglePLL}
                />
              </div>
            </div>

            <Separator />

            {/* Method Name */}
            <div className="flex items-center justify-between">
              <span className="text-sm font-medium">Method:</span>
              <Badge variant="secondary">{getMethodName(methodConfig)}</Badge>
            </div>

            <Button
              className="w-full"
              size="lg"
              onClick={handleSolve}
              disabled={isSolving || !wasmReady}
            >
              {isSolving ? (
                <>
                  <RotateCcw className="h-4 w-4 mr-2 animate-spin" />
                  Solving...
                </>
              ) : (
                <>
                  <Check className="h-4 w-4 mr-2" />
                  Solve Cube
                </>
              )}
            </Button>

            {error && (
              <div className="flex items-center gap-2 text-destructive text-sm">
                <AlertCircle className="h-4 w-4" />
                {error}
              </div>
            )}
          </CardContent>
        </Card>
      </div>

      {/* Right: Solution Output */}
      <div className="space-y-5">
        {solution ? (
          <>
            {/* Solution Summary */}
            <Card>
              <CardHeader className="pb-3">
                <div className="flex items-center justify-between">
                  <CardTitle className="text-lg">Solution</CardTitle>
                  <Badge>{solution.totalMoves} moves</Badge>
                </div>
              </CardHeader>
              <CardContent className="space-y-4">
                {/* Stage selector */}
                <div className="flex gap-2">
                  {(['cross', 'f2l', 'oll', 'pll'] as const).map((stage) => (
                    <Button
                      key={stage}
                      variant={activeStage === stage ? 'default' : 'outline'}
                      size="sm"
                      onClick={() => setActiveStage(stage)}
                      className="flex-1 capitalize"
                    >
                      <div className={`w-2 h-2 rounded-full mr-1.5 ${stageColor(stage)}`} />
                      {stage === 'f2l' ? 'F2L' : stage.toUpperCase()}
                      <span className="ml-1 text-xs opacity-70">
                        ({stage === 'cross' ? solution.crossMoves.length :
                          stage === 'f2l' ? solution.f2lMoves.length :
                          stage === 'oll' ? solution.ollMoves.length :
                          solution.pllMoves.length})
                      </span>
                    </Button>
                  ))}
                </div>

                {/* Stage moves */}
                <div className="bg-muted rounded-lg p-4">
                  <div className="flex items-center gap-1 flex-wrap">
                    {stageMoves.length > 0 ? (
                      stageMoves.map((move, i) => (
                        <span
                          key={i}
                          className="inline-flex items-center px-2 py-1 bg-background rounded text-sm font-mono font-medium shadow-sm"
                        >
                          {move}
                        </span>
                      ))
                    ) : (
                      <span className="text-sm text-muted-foreground">
                        {activeStage === 'cross' ? 'Cross already solved' : `${activeStage.toUpperCase()} already solved`}
                      </span>
                    )}
                  </div>
                </div>

                {/* Full notation */}
                <div>
                  <Label className="text-xs">Full Solution</Label>
                  <div className="mt-1 p-3 bg-muted rounded-lg text-sm font-mono">
                    {allMoves.join(' ')}
                  </div>
                </div>

                {/* Play button */}
                <Button
                  className="w-full"
                  variant="secondary"
                  onClick={() => {
                    setShowPlayer(!showPlayer);
                    setCurrentMoveIndex(0);
                  }}
                >
                  {showPlayer ? (
                    <>
                      <Pause className="h-4 w-4 mr-2" />
                      Hide Animation
                    </>
                  ) : (
                    <>
                      <Play className="h-4 w-4 mr-2" />
                      Animate Solution
                    </>
                  )}
                </Button>
              </CardContent>
            </Card>

            {/* 3D Move Player */}
            {showPlayer && (
              <MovePlayer
                moves={allMoves}
                currentMove={currentMoveIndex}
                onMoveChange={setCurrentMoveIndex}
                scramble={scrambleNotation}
              />
            )}

            {/* Stage breakdown */}
            <Card>
              <CardHeader className="pb-3">
                <CardTitle className="text-lg">Stage Breakdown</CardTitle>
              </CardHeader>
              <CardContent className="space-y-3">
                {[
                  { label: 'Cross', moves: solution.crossMoves, color: 'text-blue-500' },
                  { label: 'F2L', moves: solution.f2lMoves, color: 'text-green-500' },
                  { label: 'OLL', moves: solution.ollMoves, color: 'text-yellow-500' },
                  { label: 'PLL', moves: solution.pllMoves, color: 'text-purple-500' },
                ].map((stage) => (
                  <div key={stage.label} className="flex items-start gap-3">
                    <div className={`mt-0.5 font-semibold text-sm w-12 ${stage.color}`}>
                      {stage.label}
                    </div>
                    <div className="flex-1 flex flex-wrap gap-1">
                      {stage.moves.map((move, i) => (
                        <span
                          key={i}
                          className="text-xs font-mono bg-muted px-1.5 py-0.5 rounded"
                        >
                          {move}
                        </span>
                      ))}
                      {stage.moves.length === 0 && (
                        <span className="text-xs text-muted-foreground">(solved)</span>
                      )}
                    </div>
                    <Badge variant="outline" className="text-xs">
                      {stage.moves.length}
                    </Badge>
                  </div>
                ))}
              </CardContent>
            </Card>
          </>
        ) : (
          <Card className="border-dashed">
            <CardContent className="py-12 text-center">
              <Cuboid className="h-12 w-12 mx-auto mb-4 text-muted-foreground" />
              <p className="text-muted-foreground">
                Configure your cube and click "Solve Cube" to see the solution
              </p>
            </CardContent>
          </Card>
        )}
      </div>
    </div>
    </div>
  );
}
