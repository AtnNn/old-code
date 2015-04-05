import IO
import Char
import Control.Monad
import Control.Applicative
import System
import Data.Bits

main = do
  [a,b] <- getArgs
  putStr =<< zipWith enc <$> readFile a <*> readFile b

enc a b = chr $ xor (ord a) (ord b)