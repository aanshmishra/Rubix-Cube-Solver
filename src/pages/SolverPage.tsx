import { useState, useEffect, useCallback, useRef } from 'react';
import { useCubeStore } from '@/store/useCubeStore';
import { solveCross } from '@/lib/solver/crossSolver';
import { solveF2L } from '@/lib/solver/f2lSolver';
import { solveOLL } from '@/lib/solver/ollSolver';
import { solvePLL } from '@/lib/solver/pllSolver';
import { applyNotationToFacelets } from '@/lib/cubeEngine';
import { getSolvedFacelets } from '@/lib/orientation';
import { loadWasmSolver, toMoveArray } from '@/wasm/wasmLoader';
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
  Check,
  AlertCircle,
  Settings,
  Cuboid,
  Timer,
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
  const [currentMoveIndex, setCurrentMoveIndex] = useState(0);
  const [solveTime, setSolveTime] = useState<number | null>(null);
  const resultRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    let cancelled = false;
    loadWasmSolver()
      .then((s) => { if (!cancelled) setWasmReady(s !== null); })
      .catch(() => { if (!cancelled) setWasmReady(false); });
    return () => { cancelled = true; };
  }, []);

  // Once a solution lands, scroll the animation box up so its top tucks just
  // below the sticky header.
  useEffect(() => {
    if (solution && resultRef.current) {
      resultRef.current.scrollIntoView({ behavior: 'smooth', block: 'start' });
    }
  }, [solution]);

  const handleSolve = useCallback(async () => {
    setIsSolving(true);
    setError('');
    setSolution(null);
    setSolveTime(null);
    setCurrentMoveIndex(0);

    const startedAt = performance.now();

    try {
      // Both engines run the same staged CFOP over bidirectional BFS; the C++
      // build is roughly 5x quicker for the same move count, so it is preferred
      // whenever the WASM asset is present.
      const wasm = await loadWasmSolver();

      let crossMoves: string[];
      let f2lMoves: string[];
      let ollMoves: string[];
      let pllMoves: string[];
      let methodName: string;

      if (wasm) {
        // The solved cube for this orientation is what tells the C++ side the
        // colour scheme - it carries no orientation logic of its own.
        const target = getSolvedFacelets(orientation).join('');
        const out = wasm.solveFacelets(
          facelets.join(''),
          target,
          methodConfig.f2lAdvanced,
          methodConfig.ollAdvanced,
          methodConfig.pllAdvanced
        );

        if (!out.success) throw new Error(out.errorMessage || 'The C++ solver could not solve this cube');

        crossMoves = toMoveArray(out.crossMoves);
        f2lMoves = toMoveArray(out.f2lMoves);
        ollMoves = toMoveArray(out.ollMoves);
        pllMoves = toMoveArray(out.pllMoves);
        methodName = `${out.methodName} (C++/WASM)`;
      } else {
        let currentState = [...facelets];

        crossMoves = solveCross(currentState, orientation);
        currentState = applyNotationToFacelets(currentState, crossMoves.join(' '));

        f2lMoves = solveF2L(currentState, orientation);
        currentState = applyNotationToFacelets(currentState, f2lMoves.join(' '));

        ollMoves = await solveOLL(currentState, orientation);
        currentState = applyNotationToFacelets(currentState, ollMoves.join(' '));

        pllMoves = await solvePLL(currentState, orientation);
        methodName = `${getMethodName(methodConfig)} (TypeScript)`;
      }

      const result: SolutionResult = {
        crossMoves,
        f2lMoves,
        ollMoves,
        pllMoves,
        config: methodConfig,
        success: true,
        errorMessage: '',
        methodName,
        totalMoves: crossMoves.length + f2lMoves.length + ollMoves.length + pllMoves.length,
      };

      setSolveTime(performance.now() - startedAt);
      setSolution(result);

      // Save to history
      const faceletStr = facelets.join('');
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
      case 'cross': return 'bg-slate-400';
      case 'f2l': return 'bg-teal-600';
      case 'oll': return 'bg-amber-700';
      case 'pll': return 'bg-indigo-500';
      default: return 'bg-zinc-500';
    }
  };

  return (
    <div className="space-y-5">
      {/* Top: 2D cube map (left) + scramble & settings (right) */}
      <div className="grid grid-cols-1 gap-5 lg:grid-cols-2">
        {/* Left: 2D Cube map */}
        <Card className="gap-3">
          <CardHeader className="pb-0">
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
          <CardContent className="flex h-full flex-col justify-start">
            <CubeNet />
          </CardContent>
        </Card>

        {/* Right: Scramble + Settings */}
        <div className="space-y-5">
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
                <div className="flex items-center gap-1.5">
                  <Badge variant="secondary">{getMethodName(methodConfig)}</Badge>
                  {/* Which engine will run. The TypeScript solver is a working
                      fallback, not an error, so this is informational only. */}
                  <Badge variant="outline" className="font-mono text-[10px]">
                    {wasmReady ? 'C++/WASM' : 'TypeScript'}
                  </Badge>
                </div>
              </div>

              <Button
                className="w-full"
                size="lg"
                onClick={handleSolve}
                disabled={isSolving}
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
      </div>

      {/* Below both: 3D animation + result info beside it */}
      <div>
        {solution ? (
          <div className="grid grid-cols-1 gap-5 lg:grid-cols-[minmax(0,1.4fr)_minmax(0,1fr)]">
            {/* Animation */}
            <div ref={resultRef} className="scroll-mt-[68px]">
              <MovePlayer
                moves={allMoves}
                currentMove={currentMoveIndex}
                onMoveChange={setCurrentMoveIndex}
                scramble={scrambleNotation}
              />
            </div>

            {/* Info beside the animation */}
            <div className="space-y-5">
              {/* Finished-in banner */}
              <Card className="border-primary/40 bg-primary/5">
                <CardContent className="flex items-center justify-between py-4">
                  <div className="flex items-center gap-3">
                    <div className="flex size-10 items-center justify-center rounded-md bg-primary/15 text-primary">
                      <Timer className="h-5 w-5" />
                    </div>
                    <div>
                      <p className="text-xs uppercase tracking-wide text-muted-foreground">Solved</p>
                      <p className="text-lg font-semibold">
                        Finished in {solveTime != null ? (solveTime / 1000).toFixed(2) : '—'}s
                      </p>
                    </div>
                  </div>
                  <Badge className="text-sm">{solution.totalMoves} moves</Badge>
                </CardContent>
              </Card>

              {/* Solution summary */}
              <Card>
                <CardHeader className="pb-3">
                  <CardTitle className="text-lg">Solution</CardTitle>
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
                </CardContent>
              </Card>

              {/* Stage breakdown */}
              <Card>
                <CardHeader className="pb-3">
                  <CardTitle className="text-lg">Stage Breakdown</CardTitle>
                </CardHeader>
                <CardContent className="space-y-3">
                  {[
                    { label: 'Cross', moves: solution.crossMoves, color: 'text-slate-400' },
                    { label: 'F2L', moves: solution.f2lMoves, color: 'text-teal-500' },
                    { label: 'OLL', moves: solution.ollMoves, color: 'text-amber-600' },
                    { label: 'PLL', moves: solution.pllMoves, color: 'text-indigo-400' },
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
            </div>
          </div>
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
  );
}
