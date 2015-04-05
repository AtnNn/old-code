-- INCOMPLETE --

-- Started by Etienne Laurin

-- Replace code that doesn't type by calls to error with the error message
-- define undefined symbols as undefined

module Main where

import System.Process
import Control.Monad
import System

data Task = Task {
  file :: String,
  line :: Int,
  column :: Int,
  text :: String,
  replacement :: String
}

main :: IO ()
main = do
  (_, _, err, handle) <- createProcess $ proc "ghc" =<< getArgs
  errors <- lines <$> readAll err
  code <- waitForProcess handle
  when (code /= 1) $ staticToRuntime errors
  
staticToRuntime :: [String] -> IO ()
staticToRuntime errors = do
  tasks <- howToFix errors
  let files = groupBy file tasks
  case files of
    [] -> do
      hPutStrLn "No errors found"
    [file] -> do
      lines <- readFile file
      putStrLn $ "-- Delaying static errors to runtime in " ++ file
      putStr $ unlines $ runTasks (sortBy (liftM2 (,) line column) tasks) lines
    _ -> do
      hPutStrLn stderr $ "Too many files: " ++ concatIntersperse ", " files

runTasks :: [Tasks] -> [String] -> [String]
runTasks = run 1 0
  where
    run _ [] lines = lines
    run n c ts@(t:_) (l:ls) | n < line t = l : run (n+1) ts ls
    run n c ts@(t:_) (l:ls) | n > line t = error "error: went too far"
    run n c (t:ts) (l:ls) = (take (column t) l ++ replacement)
                            : run (n + skippedLines)
                                  (if skippedLines == 0 then c + skippedChars else skippedChars)
                                  ts
                                  (restOfLine : drop skippedLines ls)
      where (restOfLine, skippedLines, skippedChars) = skipOver 0 (text t) $ drop (column t l) : ls
            
skipOver :: String -> [String] -> (String, Int, Int)
skipOver text (l:ls) n =
  case skip text l of
  where
    skip (a:as) bs | isSpace a = skip as bs
    skip as (b:bs) | isSpace b = skip as bs
    skip (a:as) (b:bs) | a == b = skip as bs
    skip [] bs = ("", bs)
    skip as [] = (as, "")
    skip as bs = error $ "line does not match error: \n\t" ++ as ++ "\n\t" ++ bs
