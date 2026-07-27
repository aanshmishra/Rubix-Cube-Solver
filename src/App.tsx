import { useEffect } from 'react';
import { Routes, Route, Link, useLocation } from 'react-router-dom';
import { BookOpen, Cuboid, History } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { loadWasmSolver } from '@/wasm/wasmLoader';

import SolverPage from '@/pages/SolverPage';
import LibraryPage from '@/pages/LibraryPage';
import HistoryPage from '@/pages/HistoryPage';

function App() {
  const location = useLocation();

  useEffect(() => {
    // Dark mode only.
    document.documentElement.classList.add('dark');
  }, []);

  useEffect(() => {
    // Warm the C++ solver so the first solve does not pay for instantiation.
    // A failure is not fatal - SolverPage falls back to the TS solver.
    loadWasmSolver().catch(console.error);
  }, []);

  const navItems = [
    { path: '/', label: 'Solver', icon: Cuboid },
    { path: '/library', label: 'Library', icon: BookOpen },
    { path: '/history', label: 'History', icon: History },
  ];

  return (
    <div className="app-surface min-h-screen text-foreground">
      <header className="sticky top-0 z-50 border-b bg-card/90 backdrop-blur-md">
        <div className="mx-auto flex h-14 max-w-7xl items-center justify-between px-3 sm:px-5">
          <div className="flex min-w-0 items-center gap-2">
            <div className="flex size-8 items-center justify-center rounded-md border bg-background">
              <Cuboid className="h-5 w-5 text-primary" />
            </div>
            <h1 className="hidden text-lg font-bold tracking-tight sm:block">
              Rubix Cube Solver
            </h1>
          </div>

          <nav className="flex items-center gap-1 rounded-lg border bg-background/70 p-1">
            {navItems.map((item) => {
              const Icon = item.icon;
              const isActive = location.pathname === item.path;
              return (
                <Link key={item.path} to={item.path}>
                  <Button
                    variant={isActive ? 'secondary' : 'ghost'}
                    size="sm"
                    className={isActive ? 'gap-1.5 shadow-sm' : 'gap-1.5 text-muted-foreground'}
                  >
                    <Icon className="h-4 w-4" />
                    <span className="hidden sm:inline">{item.label}</span>
                  </Button>
                </Link>
              );
            })}
          </nav>
        </div>
      </header>

      <main className="panel-grid mx-auto min-h-[calc(100vh-7rem)] max-w-7xl px-3 py-5 sm:px-5 sm:py-6">
        <Routes>
          <Route path="/" element={<SolverPage />} />
          <Route path="/library" element={<LibraryPage />} />
          <Route path="/history" element={<HistoryPage />} />
        </Routes>
      </main>

      <footer className="border-t py-4">
        <div className="mx-auto flex max-w-7xl items-center justify-center gap-2 px-5 text-xs text-muted-foreground">
          <span>Rubix Cube Solver</span>
        </div>
      </footer>
    </div>
  );
}

export default App;
