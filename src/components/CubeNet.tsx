import { useCubeStore } from '@/store/useCubeStore';
import { FACE_COLORS, FACE_COLORS_DARK, type FaceColor } from '@/types';
import { useThemeStore } from '@/store/useThemeStore';
import { cn } from '@/lib/utils';

// Standard cube net layout
//      [U]
// [L]  [F]  [R]  [B]
//      [D]

const NET_LAYOUT = [
  // [row, col, faceIndex]
  [0, 1, 0], // U
  [1, 0, 4], // L
  [1, 1, 2], // F
  [1, 2, 1], // R
  [1, 3, 5], // B
  [2, 1, 3], // D
] as const;

export default function CubeNet() {
  const { facelets, selectedColor, setSelectedColor, setFacelets } = useCubeStore();
  const { theme } = useThemeStore();
  const colors = theme === 'dark' ? FACE_COLORS_DARK : FACE_COLORS;

  const handleStickerClick = (faceIndex: number, stickerIndex: number) => {
    const newFacelets = [...facelets];
    newFacelets[faceIndex * 9 + stickerIndex] = selectedColor;
    setFacelets(newFacelets);
  };

  const colorOptions: { color: FaceColor; label: string }[] = [
    { color: 'W', label: 'White' },
    { color: 'R', label: 'Red' },
    { color: 'G', label: 'Green' },
    { color: 'Y', label: 'Yellow' },
    { color: 'O', label: 'Orange' },
    { color: 'B', label: 'Blue' },
  ];

  return (
    <div className="space-y-4">
      {/* Color selector */}
      <div className="flex items-center gap-2 flex-wrap">
        <span className="text-xs font-medium text-muted-foreground mr-1">Paint:</span>
        {colorOptions.map(({ color, label }) => (
          <button
            key={color}
            onClick={() => setSelectedColor(color)}
            className={cn(
              'w-8 h-8 rounded-md border-2 transition-all sticker-btn',
              selectedColor === color ? 'border-primary scale-110 ring-2 ring-primary/30' : 'border-transparent'
            )}
            style={{ backgroundColor: colors[color] }}
            title={label}
          />
        ))}
      </div>

      {/* Net grid */}
      <div className="inline-block">
        <div
          className="grid gap-1"
          style={{
            gridTemplateColumns: 'repeat(4, 1fr)',
            gridTemplateRows: 'repeat(3, 1fr)',
          }}
        >
          {NET_LAYOUT.map(([row, col, faceIndex]) => (
            <div
              key={faceIndex}
              className="grid grid-cols-3 gap-0.5 p-1 rounded-lg bg-muted/50"
              style={{
                gridColumn: col + 1,
                gridRow: row + 1,
              }}
            >
              {Array.from({ length: 9 }, (_, i) => (
                <button
                  key={i}
                  onClick={() => handleStickerClick(faceIndex, i)}
                  className={cn(
                    'w-10 h-10 sm:w-12 sm:h-12 rounded-sm sticker-btn transition-all',
                    i === 4 && 'ring-1 ring-foreground/20' // Center piece
                  )}
                  style={{
                    backgroundColor: colors[facelets[faceIndex * 9 + i]],
                  }}
                />
              ))}
            </div>
          ))}
        </div>
      </div>

      <p className="text-xs text-muted-foreground">
        Click a color above, then click stickers to paint the cube. Centers define the color scheme.
      </p>
    </div>
  );
}
