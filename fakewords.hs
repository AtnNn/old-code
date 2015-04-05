import System.IO
import qualified Data.Map as M
import Data.List
import Control.Monad
import System.Random
import Data.Maybe
import System.IO.Unsafe
import Data.Char

main = mapM_ putStrLn =<< genWords . prep =<< getContents

genWords :: M.Map String String -> IO [String]
genWords m = sequence $ take 10 $ repeat $ unsafeInterleaveIO $ genWord m

prep = M.fromList . 
       map (\l -> (fst (head l), map snd l)) .
       groupBy ((. fst) . (==) . fst) .
       sort .
       map (\[a,b,c] -> ([a,b], c)) .
       triplets .
       (' ':) . (++" ")
       
triplets (a:l@(b:c:_)) = [a,b,c] : triplets l
triplets _ = []

genWord m = do
  f <- start m
  nextw m f
  
start m = do
  let ks = filter (isSpace . head) $ M.keys m
  fmap (ks !!) $ randomRIO (0,length ks - 1)
  
nextw m f = do
  let k = reverse $ take 2 $ reverse f
      n = fromMaybe " " $ M.lookup k m
  w <- fmap (n !!) $ randomRIO (0, length n - 1)
  if w == ' ' then return $ tail f else nextw m (f ++ [w])
