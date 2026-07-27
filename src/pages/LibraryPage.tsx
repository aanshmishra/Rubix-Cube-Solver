import { useState, useEffect } from 'react';
import { Card, CardContent } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Badge } from '@/components/ui/badge';
import { Checkbox } from '@/components/ui/checkbox';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { Progress } from '@/components/ui/progress';
import {
  BookOpen,
  Search,
  CheckCircle2,
  RotateCcw,
  AlertCircle,
} from 'lucide-react';
import type { AlgorithmCase } from '@/types';
import {
  toggleOLLLearned,
  togglePLLLearned,
  isOLLLearned,
  isPLLLearned,
  getProgressStats,
} from '@/storage/history';

// Import algorithm data
import ollData from '../../cpp/data/oll_cases.json';
import pllData from '../../cpp/data/pll_cases.json';

// TypeScript widens every string in an imported JSON file to `string`, so the
// `difficulty` field will not narrow to its union on its own. These files are
// checked in beside the code, so asserting the shape at the boundary is safe.
const OLL_CASES = ollData.cases as AlgorithmCase[];
const PLL_CASES = pllData.cases as AlgorithmCase[];

type Difficulty = 'easy' | 'medium' | 'hard';

const DIFFICULTY_COLORS: Record<Difficulty, string> = {
  easy: 'bg-green-500',
  medium: 'bg-yellow-500',
  hard: 'bg-red-500',
};

export default function LibraryPage() {
  const [search, setSearch] = useState('');
  const [ollCases, setOllCases] = useState<AlgorithmCase[]>(OLL_CASES);
  const [pllCases, setPllCases] = useState<AlgorithmCase[]>(PLL_CASES);
  const [progress, setProgress] = useState(getProgressStats());
  const [activeTab, setActiveTab] = useState('oll');

  useEffect(() => {
    const filteredOll = OLL_CASES.filter(
      (c) =>
        c.name.toLowerCase().includes(search.toLowerCase()) ||
        c.algorithm.toLowerCase().includes(search.toLowerCase()) ||
        c.id.toString().includes(search)
    );
    const filteredPll = PLL_CASES.filter(
      (c) =>
        c.name.toLowerCase().includes(search.toLowerCase()) ||
        c.algorithm.toLowerCase().includes(search.toLowerCase()) ||
        c.id.toString().includes(search)
    );
    setOllCases(filteredOll);
    setPllCases(filteredPll);
  }, [search]);

  const handleToggleOLL = (id: number) => {
    toggleOLLLearned(id);
    setProgress(getProgressStats());
  };

  const handleTogglePLL = (id: number) => {
    togglePLLLearned(id);
    setProgress(getProgressStats());
  };

  const renderCaseCard = (alg: AlgorithmCase, type: 'oll' | 'pll') => {
    const isLearned = type === 'oll' ? isOLLLearned(alg.id) : isPLLLearned(alg.id);
    const onToggle = type === 'oll' ? () => handleToggleOLL(alg.id) : () => handleTogglePLL(alg.id);

    return (
      <div
        key={`${type}-${alg.id}`}
        className={`p-4 border rounded-lg transition-all hover:shadow-md ${
          isLearned ? 'bg-primary/5 border-primary/20' : ''
        }`}
      >
        <div className="flex items-start gap-3">
          <Checkbox
            checked={isLearned}
            onCheckedChange={onToggle}
            className="mt-1"
          />
          <div className="flex-1 min-w-0">
            <div className="flex items-center gap-2 mb-1">
              <span className="font-mono text-sm text-muted-foreground">
                #{alg.id}
              </span>
              <span className="font-semibold text-sm">{alg.name}</span>
              {alg.difficulty !== 'none' && (
                <Badge
                  variant="outline"
                  className={`text-xs ${DIFFICULTY_COLORS[alg.difficulty as Difficulty]} text-white border-0`}
                >
                  {alg.difficulty}
                </Badge>
              )}
              {isLearned && (
                <CheckCircle2 className="h-4 w-4 text-green-500 ml-auto" />
              )}
            </div>

            {alg.cycle && (
              <p className="text-xs text-muted-foreground mb-1">{alg.cycle}</p>
            )}
            {alg.type && (
              <Badge variant="secondary" className="text-xs mb-2">
                {alg.type}
              </Badge>
            )}

            <p className="font-mono text-sm bg-muted p-2 rounded mt-1">
              {alg.algorithm}
            </p>
          </div>
        </div>
      </div>
    );
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h2 className="text-2xl font-bold flex items-center gap-2">
            <BookOpen className="h-6 w-6" />
            Algorithm Library
          </h2>
          <p className="text-muted-foreground">
            Learn and track your progress with OLL and PLL algorithms
          </p>
        </div>
      </div>

      {/* Progress Overview */}
      <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
        <Card>
          <CardContent className="pt-6">
            <div className="flex items-center justify-between mb-2">
              <span className="text-sm font-medium">OLL Progress</span>
              <span className="text-sm text-muted-foreground">
                {progress.ollLearned} / {progress.ollTotal}
              </span>
            </div>
            <Progress value={(progress.ollLearned / progress.ollTotal) * 100} />
            <p className="text-xs text-muted-foreground mt-2">
              {Math.round((progress.ollLearned / progress.ollTotal) * 100)}% complete
            </p>
          </CardContent>
        </Card>
        <Card>
          <CardContent className="pt-6">
            <div className="flex items-center justify-between mb-2">
              <span className="text-sm font-medium">PLL Progress</span>
              <span className="text-sm text-muted-foreground">
                {progress.pllLearned} / {progress.pllTotal}
              </span>
            </div>
            <Progress value={(progress.pllLearned / progress.pllTotal) * 100} />
            <p className="text-xs text-muted-foreground mt-2">
              {Math.round((progress.pllLearned / progress.pllTotal) * 100)}% complete
            </p>
          </CardContent>
        </Card>
      </div>

      {/* Search */}
      <div className="relative">
        <Search className="absolute left-3 top-1/2 -translate-y-1/2 h-4 w-4 text-muted-foreground" />
        <Input
          placeholder="Search algorithms by name, notation, or ID..."
          value={search}
          onChange={(e) => setSearch(e.target.value)}
          className="pl-10"
        />
      </div>

      {/* Tabs */}
      <Tabs value={activeTab} onValueChange={setActiveTab}>
        <TabsList className="grid w-full grid-cols-2">
          <TabsTrigger value="oll">
            OLL ({ollCases.length}/{OLL_CASES.length})
          </TabsTrigger>
          <TabsTrigger value="pll">
            PLL ({pllCases.length}/{PLL_CASES.length})
          </TabsTrigger>
        </TabsList>

        <TabsContent value="oll" className="mt-4">
          <div className="space-y-2">
            <div className="flex items-center justify-between text-sm text-muted-foreground mb-3">
              <span>{ollData.description}</span>
              <Button
                variant="ghost"
                size="sm"
                onClick={() => setSearch('')}
              >
                <RotateCcw className="h-3.5 w-3.5 mr-1" />
                Clear filter
              </Button>
            </div>
            {ollCases.length > 0 ? (
              ollCases.map((alg) => renderCaseCard(alg, 'oll'))
            ) : (
              <div className="text-center py-12 text-muted-foreground">
                <AlertCircle className="h-8 w-8 mx-auto mb-2" />
                <p>No OLL cases match your search</p>
              </div>
            )}
          </div>
        </TabsContent>

        <TabsContent value="pll" className="mt-4">
          <div className="space-y-2">
            <div className="flex items-center justify-between text-sm text-muted-foreground mb-3">
              <span>{pllData.description}</span>
              <Button
                variant="ghost"
                size="sm"
                onClick={() => setSearch('')}
              >
                <RotateCcw className="h-3.5 w-3.5 mr-1" />
                Clear filter
              </Button>
            </div>
            {pllCases.length > 0 ? (
              pllCases.map((alg) => renderCaseCard(alg, 'pll'))
            ) : (
              <div className="text-center py-12 text-muted-foreground">
                <AlertCircle className="h-8 w-8 mx-auto mb-2" />
                <p>No PLL cases match your search</p>
              </div>
            )}
          </div>
        </TabsContent>
      </Tabs>
    </div>
  );
}
