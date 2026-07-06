import { useState, useEffect, useRef } from 'react';
import { Button } from '@/components/ui/button';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Slider } from '@/components/ui/slider';
import {
  Play,
  Pause,
  SkipBack,
  SkipForward,
  RotateCcw,
  Gauge,
} from 'lucide-react';
import Scene3D from '@/three/Scene3D';

interface MovePlayerProps {
  moves: string[];
  currentMove: number;
  onMoveChange: (index: number) => void;
  scramble?: string;
}

export default function MovePlayer({ moves, currentMove, onMoveChange, scramble }: MovePlayerProps) {
  const [isPlaying, setIsPlaying] = useState(false);
  const [speed, setSpeed] = useState([500]); // ms between moves
  const timerRef = useRef<ReturnType<typeof setTimeout> | null>(null);

  useEffect(() => {
    if (isPlaying && currentMove < moves.length) {
      timerRef.current = setTimeout(() => {
        onMoveChange(currentMove + 1);
      }, speed[0]);
    } else if (currentMove >= moves.length) {
      setIsPlaying(false);
    }

    return () => {
      if (timerRef.current) {
        clearTimeout(timerRef.current);
      }
    };
  }, [isPlaying, currentMove, moves.length, speed, onMoveChange]);

  const handlePlayPause = () => {
    if (currentMove >= moves.length) {
      onMoveChange(0);
    }
    setIsPlaying(!isPlaying);
  };

  const handleReset = () => {
    setIsPlaying(false);
    onMoveChange(0);
  };

  const handleStepForward = () => {
    if (currentMove < moves.length) {
      onMoveChange(currentMove + 1);
    }
  };

  const handleStepBack = () => {
    if (currentMove > 0) {
      setIsPlaying(false);
      onMoveChange(currentMove - 1);
    }
  };

  // Get the moves played so far
  const playedMoves = moves.slice(0, currentMove);

  return (
    <Card>
      <CardHeader className="pb-3">
        <CardTitle className="text-lg flex items-center justify-between">
          <span>3D Animation</span>
          <span className="text-sm font-normal text-muted-foreground">
            {currentMove} / {moves.length}
          </span>
        </CardTitle>
      </CardHeader>
      <CardContent className="space-y-4">
        {/* 3D View */}
        <div className="aspect-video bg-muted rounded-lg overflow-hidden">
          <Scene3D moves={playedMoves} scramble={scramble} />
        </div>

        {/* Move list with current position */}
        <div className="bg-muted rounded-lg p-3 max-h-24 overflow-y-auto">
          <div className="flex flex-wrap gap-1">
            {moves.map((move, i) => (
              <span
                key={i}
                className={`inline-flex items-center px-1.5 py-0.5 rounded text-xs font-mono font-medium transition-colors ${
                  i < currentMove
                    ? 'bg-primary text-primary-foreground'
                    : i === currentMove
                    ? 'bg-yellow-500 text-black ring-2 ring-yellow-300'
                    : 'bg-background text-muted-foreground'
                }`}
              >
                {move}
              </span>
            ))}
          </div>
        </div>

        {/* Controls */}
        <div className="flex items-center gap-2">
          <Button variant="outline" size="sm" onClick={handleReset}>
            <RotateCcw className="h-4 w-4" />
          </Button>
          <Button variant="outline" size="sm" onClick={handleStepBack}>
            <SkipBack className="h-4 w-4" />
          </Button>
          <Button onClick={handlePlayPause} size="sm" className="flex-1">
            {isPlaying ? (
              <Pause className="h-4 w-4 mr-1" />
            ) : (
              <Play className="h-4 w-4 mr-1" />
            )}
            {isPlaying ? 'Pause' : currentMove >= moves.length ? 'Replay' : 'Play'}
          </Button>
          <Button variant="outline" size="sm" onClick={handleStepForward}>
            <SkipForward className="h-4 w-4" />
          </Button>
        </div>

        {/* Speed control */}
        <div className="flex items-center gap-3">
          <Gauge className="h-4 w-4 text-muted-foreground" />
          <Slider
            value={speed}
            onValueChange={setSpeed}
            min={100}
            max={2000}
            step={100}
            className="flex-1"
          />
          <span className="text-xs text-muted-foreground w-16 text-right">
            {speed[0]}ms
          </span>
        </div>
      </CardContent>
    </Card>
  );
}
