import type { SolveHistoryEntry, MethodConfig } from '@/types';

const HISTORY_KEY = 'rubiks_solver_history';
const PROGRESS_KEY = 'rubiks_solver_progress';

// ---------------------------------------------------------------------------
// Solve History
// ---------------------------------------------------------------------------

export function saveSolveEntry(entry: SolveHistoryEntry): void {
  try {
    const history = getSolveHistory();
    history.unshift(entry); // Add to beginning
    // Keep max 100 entries
    if (history.length > 100) {
      history.pop();
    }
    localStorage.setItem(HISTORY_KEY, JSON.stringify(history));
  } catch (e) {
    console.error('Failed to save solve history:', e);
  }
}

export function getSolveHistory(): SolveHistoryEntry[] {
  try {
    const data = localStorage.getItem(HISTORY_KEY);
    return data ? JSON.parse(data) : [];
  } catch (e) {
    console.error('Failed to load solve history:', e);
    return [];
  }
}

export function clearSolveHistory(): void {
  localStorage.removeItem(HISTORY_KEY);
}

export function deleteSolveEntry(id: string): void {
  try {
    const history = getSolveHistory().filter((e) => e.id !== id);
    localStorage.setItem(HISTORY_KEY, JSON.stringify(history));
  } catch (e) {
    console.error('Failed to delete solve entry:', e);
  }
}

export function createSolveEntry(
  scramble: string,
  scrambleNotation: string,
  methodName: string,
  methodConfig: MethodConfig,
  crossMoves: number,
  f2lMoves: number,
  ollMoves: number,
  pllMoves: number,
  solutionNotation: string
): SolveHistoryEntry {
  return {
    id: `${Date.now()}-${Math.random().toString(36).substr(2, 9)}`,
    timestamp: Date.now(),
    scramble,
    scrambleNotation,
    methodName,
    methodConfig,
    totalMoves: crossMoves + f2lMoves + ollMoves + pllMoves,
    crossMoves,
    f2lMoves,
    ollMoves,
    pllMoves,
    solutionNotation,
  };
}

// ---------------------------------------------------------------------------
// Learning Progress (OLL/PLL checkbox tracking)
// ---------------------------------------------------------------------------

export interface LearningProgress {
  ollLearned: number[]; // IDs of learned OLL cases
  pllLearned: number[]; // IDs of learned PLL cases
}

export function getLearningProgress(): LearningProgress {
  try {
    const data = localStorage.getItem(PROGRESS_KEY);
    return data ? JSON.parse(data) : { ollLearned: [], pllLearned: [] };
  } catch (e) {
    console.error('Failed to load learning progress:', e);
    return { ollLearned: [], pllLearned: [] };
  }
}

export function saveLearningProgress(progress: LearningProgress): void {
  try {
    localStorage.setItem(PROGRESS_KEY, JSON.stringify(progress));
  } catch (e) {
    console.error('Failed to save learning progress:', e);
  }
}

export function toggleOLLLearned(id: number): void {
  const progress = getLearningProgress();
  const idx = progress.ollLearned.indexOf(id);
  if (idx >= 0) {
    progress.ollLearned.splice(idx, 1);
  } else {
    progress.ollLearned.push(id);
  }
  saveLearningProgress(progress);
}

export function togglePLLLearned(id: number): void {
  const progress = getLearningProgress();
  const idx = progress.pllLearned.indexOf(id);
  if (idx >= 0) {
    progress.pllLearned.splice(idx, 1);
  } else {
    progress.pllLearned.push(id);
  }
  saveLearningProgress(progress);
}

export function isOLLLearned(id: number): boolean {
  return getLearningProgress().ollLearned.includes(id);
}

export function isPLLLearned(id: number): boolean {
  return getLearningProgress().pllLearned.includes(id);
}

export function getProgressStats(): { ollTotal: number; ollLearned: number; pllTotal: number; pllLearned: number } {
  const progress = getLearningProgress();
  return {
    ollTotal: 57,
    ollLearned: progress.ollLearned.length,
    pllTotal: 21,
    pllLearned: progress.pllLearned.length,
  };
}

// ---------------------------------------------------------------------------
// Import/Export
// ---------------------------------------------------------------------------

export function exportAllData(): string {
  const data = {
    history: getSolveHistory(),
    progress: getLearningProgress(),
    exportedAt: new Date().toISOString(),
  };
  return JSON.stringify(data, null, 2);
}

export function importAllData(json: string): boolean {
  try {
    const data = JSON.parse(json);
    if (data.history) {
      localStorage.setItem(HISTORY_KEY, JSON.stringify(data.history));
    }
    if (data.progress) {
      localStorage.setItem(PROGRESS_KEY, JSON.stringify(data.progress));
    }
    return true;
  } catch (e) {
    console.error('Failed to import data:', e);
    return false;
  }
}
