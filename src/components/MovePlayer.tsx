import { useState, useEffect, useRef, useCallback } from 'react';
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
  const [speed, setSpeed] = useState([400]); // ms per move animation
  const isPlayingRef = useRef(false);
  const currentMoveRef = useRef(currentMove);

  // Keep refs in sync
  useEffect(() => {
    isPlayingRef.current = isPlaying;
  }, [isPlaying]);

  useEffect(() => {
    currentMoveRef.current = currentMove;
  }, [currentMove]);

  // When an animation finishes and we're in auto-play mode, advance to next move
  const handleAnimationComplete = useCallback(() => {
    if (isPlayingRef.current && currentMoveRef.current < moves.length) {
      // Small delay before the next move for visual breathing room
      setTimeout(() => {
        if (isPlayingRef.current) {
          onMoveChange(currentMoveRef.current + 1);
        }
      }, 80);
    } else if (currentMoveRef.current >= moves.length) {
      setIsPlaying(false);
    }
  }, [moves.length, onMoveChange]);

  // When play starts, kick off the first move advance if needed
  useEffect(() => {
    if (isPlaying && currentMove < moves.length) {
      // If we just started playing and currentMove hasn't advanced yet,
      // the Scene3D will animate the current move and call onAnimationComplete.
      // But if currentMove is 0 and no animation is happening, we need to kick it off.
      if (currentMove === 0 && moves.length > 0) {
        onMoveChange(1); // Start the first move animation
      }
    }
  }, [isPlaying]); // eslint-disable-line react-hooks/exhaustive-deps

  // Stop playing when all moves are done
  useEffect(() => {
    if (currentMove >= moves.length && moves.length > 0) {
      setIsPlaying(false);
    }
  }, [currentMove, moves.length]);

  const handlePlayPause = () => {
    if (currentMove >= moves.length) {
      // Replay from start
      onMoveChange(0);
      // Use a small timeout to let the state reset before starting play
      setTimeout(() => {
        setIsPlaying(true);
      }, 50);
      return;
    }
    if (!isPlaying && currentMove < moves.length) {
      setIsPlaying(true);
      // If not yet started, kick off the first move
      if (currentMove === 0 && moves.length > 0) {
        onMoveChange(1);
      } else {
        // Already mid-way, advance to trigger the next animation
        onMoveChange(currentMove + 1);
      }
    } else {
      setIsPlaying(false);
    }
  };

  const handleReset = () => {
    setIsPlaying(false);
    onMoveChange(0);
  };

  const handleStepForward = () => {
    if (currentMove < moves.length) {
      setIsPlaying(false);
      onMoveChange(currentMove + 1);
    }
  };

  const handleStepBack = () => {
    if (currentMove > 0) {
      setIsPlaying(false);
      onMoveChange(currentMove - 1);
    }
  };

  // Get the moves played so far (for Scene3D)
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
          <Scene3D
            moves={playedMoves}
            scramble={scramble}
            animationDuration={speed[0]}
            onAnimationComplete={handleAnimationComplete}
          />
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
                    ? 'bg-accent text-accent-foreground ring-2 ring-ring/50'
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
            max={1500}
            step={50}
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
