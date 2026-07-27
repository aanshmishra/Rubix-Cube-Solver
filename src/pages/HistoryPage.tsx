import { useState, useEffect } from 'react';
import { Card, CardContent } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import {
  History,
  Trash2,
  Download,
  Upload,
  Calendar,
  Hash,
  Layers,
  AlertCircle,
} from 'lucide-react';
import type { SolveHistoryEntry } from '@/types';
import {
  getSolveHistory,
  deleteSolveEntry,
  clearSolveHistory,
  exportAllData,
  importAllData,
} from '@/storage/history';

export default function HistoryPage() {
  const [history, setHistory] = useState<SolveHistoryEntry[]>([]);
  const [importError, setImportError] = useState('');
  const [importSuccess, setImportSuccess] = useState(false);

  useEffect(() => {
    loadHistory();
  }, []);

  const loadHistory = () => {
    setHistory(getSolveHistory());
  };

  const handleDelete = (id: string) => {
    deleteSolveEntry(id);
    loadHistory();
  };

  const handleClearAll = () => {
    if (window.confirm('Are you sure you want to clear all history? This cannot be undone.')) {
      clearSolveHistory();
      loadHistory();
    }
  };

  const handleExport = () => {
    const data = exportAllData();
    const blob = new Blob([data], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `rubiks-solver-backup-${new Date().toISOString().split('T')[0]}.json`;
    a.click();
    URL.revokeObjectURL(url);
  };

  const handleImport = (event: React.ChangeEvent<HTMLInputElement>) => {
    setImportError('');
    setImportSuccess(false);
    const file = event.target.files?.[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = (e) => {
      try {
        const content = e.target?.result as string;
        const success = importAllData(content);
        if (success) {
          setImportSuccess(true);
          loadHistory();
        } else {
          setImportError('Invalid backup file format');
        }
      } catch {
        setImportError('Failed to read file');
      }
    };
    reader.readAsText(file);
  };

  const formatDate = (timestamp: number) => {
    return new Date(timestamp).toLocaleString();
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h2 className="text-2xl font-bold flex items-center gap-2">
            <History className="h-6 w-6" />
            Solve History
          </h2>
          <p className="text-muted-foreground">
            Track your past solves and performance
          </p>
        </div>
        <div className="flex items-center gap-2">
          <Button variant="outline" size="sm" onClick={handleExport}>
            <Download className="h-4 w-4 mr-1" />
            Export
          </Button>
          <Button variant="outline" size="sm" asChild>
            <label className="cursor-pointer">
              <Upload className="h-4 w-4 mr-1" />
              Import
              <input
                type="file"
                accept=".json"
                className="hidden"
                onChange={handleImport}
              />
            </label>
          </Button>
          {history.length > 0 && (
            <Button variant="destructive" size="sm" onClick={handleClearAll}>
              <Trash2 className="h-4 w-4 mr-1" />
              Clear All
            </Button>
          )}
        </div>
      </div>

      {importSuccess && (
        <div className="bg-green-500/10 text-green-600 p-3 rounded-lg text-sm">
          Data imported successfully!
        </div>
      )}
      {importError && (
        <div className="bg-destructive/10 text-destructive p-3 rounded-lg text-sm flex items-center gap-2">
          <AlertCircle className="h-4 w-4" />
          {importError}
        </div>
      )}

      {/* Stats Summary */}
      {history.length > 0 && (
        <Card>
          <CardContent className="pt-6">
            <div className="grid grid-cols-2 sm:grid-cols-4 gap-4">
              <div className="text-center">
                <div className="text-2xl font-bold">{history.length}</div>
                <div className="text-xs text-muted-foreground">Total Solves</div>
              </div>
              <div className="text-center">
                <div className="text-2xl font-bold">
                  {Math.round(history.reduce((a, b) => a + b.totalMoves, 0) / history.length)}
                </div>
                <div className="text-xs text-muted-foreground">Avg Moves</div>
              </div>
              <div className="text-center">
                <div className="text-2xl font-bold">
                  {Math.min(...history.map((h) => h.totalMoves))}
                </div>
                <div className="text-xs text-muted-foreground">Best Moves</div>
              </div>
              <div className="text-center">
                <div className="text-2xl font-bold">
                  {new Set(history.map((h) => h.methodName)).size}
                </div>
                <div className="text-xs text-muted-foreground">Methods Used</div>
              </div>
            </div>
          </CardContent>
        </Card>
      )}

      {/* History List */}
      {history.length === 0 ? (
        <Card className="border-dashed">
          <CardContent className="py-12 text-center">
            <History className="h-12 w-12 mx-auto mb-4 text-muted-foreground" />
            <p className="text-muted-foreground">
              No solve history yet. Go to the Solver page to start solving!
            </p>
          </CardContent>
        </Card>
      ) : (
        <div className="space-y-3">
          {history.map((entry) => (
            <Card key={entry.id} className="hover:shadow-md transition-shadow">
              <CardContent className="p-4">
                <div className="flex items-start justify-between">
                  <div className="flex-1 min-w-0">
                    <div className="flex items-center gap-2 mb-2">
                      <Calendar className="h-3.5 w-3.5 text-muted-foreground" />
                      <span className="text-xs text-muted-foreground">
                        {formatDate(entry.timestamp)}
                      </span>
                      <Badge variant="secondary" className="text-xs">
                        {entry.methodName}
                      </Badge>
                    </div>

                    <div className="flex items-center gap-4 text-sm mb-2">
                      <span className="flex items-center gap-1">
                        <Hash className="h-3.5 w-3.5 text-muted-foreground" />
                        {entry.totalMoves} moves
                      </span>
                      <span className="flex items-center gap-1">
                        <Layers className="h-3.5 w-3.5 text-blue-500" />
                        {entry.crossMoves}
                      </span>
                      <span className="flex items-center gap-1">
                        <Layers className="h-3.5 w-3.5 text-green-500" />
                        {entry.f2lMoves}
                      </span>
                      <span className="flex items-center gap-1">
                        <Layers className="h-3.5 w-3.5 text-yellow-500" />
                        {entry.ollMoves}
                      </span>
                      <span className="flex items-center gap-1">
                        <Layers className="h-3.5 w-3.5 text-purple-500" />
                        {entry.pllMoves}
                      </span>
                    </div>

                    {entry.scrambleNotation && (
                      <p className="text-xs text-muted-foreground font-mono mb-1">
                        Scramble: {entry.scrambleNotation}
                      </p>
                    )}

                    {entry.solutionNotation && (
                      <p className="text-xs font-mono bg-muted p-2 rounded mt-2 overflow-x-auto">
                        {entry.solutionNotation}
                      </p>
                    )}
                  </div>

                  <Button
                    variant="ghost"
                    size="sm"
                    onClick={() => handleDelete(entry.id)}
                    className="ml-2 text-muted-foreground hover:text-destructive"
                  >
                    <Trash2 className="h-4 w-4" />
                  </Button>
                </div>
              </CardContent>
            </Card>
          ))}
        </div>
      )}
    </div>
  );
}
