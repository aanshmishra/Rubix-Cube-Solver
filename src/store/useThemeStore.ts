import { create } from 'zustand';

export type Theme = 'light' | 'dark';

interface ThemeStore {
  theme: Theme;
  toggleTheme: () => void;
  setTheme: (theme: Theme) => void;
}

// Dark mode only — the app no longer exposes a light theme.
export const useThemeStore = create<ThemeStore>((set) => ({
  theme: 'dark',
  toggleTheme: () => set({ theme: 'dark' }),
  setTheme: () => set({ theme: 'dark' }),
}));
