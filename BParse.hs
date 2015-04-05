-- Backtracking Parser for Haskell, using Monads and Continuations
-- Written by Etienne Laurin

-- examples:
--
-- BParse> runBParse (list integer (char ',')) "1,2,3,67,234"
-- Just ([1,2,3,67,234],"")
--
-- *BParse> runBParse (many (string "ba") `and` string "backtrack") "bababacktrack"
-- Just ((["ba","ba"],"backtrack"),"")
-- 

-- <AtnNn> oui bon, quand je suis fache je me confonds
-- <xsimo> qu'est-ce qu'ya ? pourquoi t'es fache
-- <AtnNn> par ce qu'on parle de java

module BParse where

import Prelude hiding (or, maybe, repeat, and)
import Data.Char

newtype BParse a b c = BParse { bparse :: (c -> a -> Maybe (b,a)) -> a -> Maybe (b,a) }

instance Monad (BParse a b) where
    return a = BParse $ \c x -> c a x
    (BParse p) >>= f = BParse $ \c x -> p (\a -> bparse (f a) c) x

get :: BParse [a] b a
get = BParse f where f c (x:xs) = c x xs; f _ _ = Nothing 

peek :: BParse [a] b a
peek = BParse f where f c xs@(x:_) = c x xs; f _ _ = Nothing

single :: (a -> Bool) -> BParse [a] b a
single f = do a <- get; if f a then return a else nothing

range :: Ord a => a -> a -> BParse [a] b a
range a b = single f where f c = c >= a && c <= b

or :: BParse a b c -> BParse a b c -> BParse a b c
(BParse f) `or` (BParse g) = BParse p
    where p c x = case f c x of
		Nothing -> g c x
		a -> a

char :: Eq a => a -> BParse [a] b a
char c = single (==c)

string :: Eq a => [a] -> BParse [a] b [a]
string = sequence . map char

insensitive :: [Char] -> BParse [Char] b [Char]
insensitive = sequence . map caseless

caseless :: Char -> BParse [Char] b Char
caseless c = or (char $ toLower c) (char $ toUpper c)

many :: BParse a b c -> BParse a b [c]
many f = BParse p
    where p c x = case bparse f (more (c . reverse) []) x of
		  Nothing -> Nothing
		  a -> a
	  more c l a x = case bparse f (more c (a:l)) x of
		       Nothing -> c (a:l) x
		       a -> a

few :: BParse a b c -> BParse a b [c]
few f = or (f >>= return . return) $ do x <- f; xs <- few f; return (x:xs)

alpha :: BParse [Char] b Char
alpha = or (range 'a' 'z') (range 'A' 'Z')

digit :: BParse [Char] b Char
digit = range '0' '9'

spaces :: BParse [Char] b Char
spaces = foldl1 or [char ' ', char '\t', char '\n', char '\r']

nothing :: BParse a b c
nothing = BParse $ \_ _ -> Nothing

integer :: BParse [Char] b Int
integer = many digit >>= return . read

maybe :: BParse a b c -> BParse a b (Maybe c)
maybe (BParse f) = BParse p
    where p c x = case f (c . Just) x of
		  Nothing -> c Nothing x
		  a -> a

tokens :: BParse a b c -> BParse a b d -> BParse a b [c]
tokens f g = many (maybe g >> f)

oneOf :: [BParse a b c] -> BParse a b c
oneOf = foldl or nothing

and :: BParse a b c -> BParse a b d -> BParse a b (c,d)
and f g = do a <- f
	     b <- g
	     return (a,b)

list :: BParse a b c -> BParse a b d -> BParse a b [c]
list elem sep = flip or (return []) $
		do x <- elem
		   xs <- maybe $ many $ sep >> elem
		   return $ x:(case xs of Nothing -> []; Just xs -> xs)

exactly :: Int -> BParse a b c -> BParse a b [c]
exactly 0 _ = return []
exactly n f = do x <- f
		 xs <- exactly (n-1) f
		 return (x:xs)

downFrom :: Int -> BParse a b c -> BParse a b [c]
downFrom 0 _ = return []
downFrom n f = flip or (return []) $ do x <- f; xs <- downFrom (n-1) f; return (x:xs)

upTo :: Int -> BParse a b c -> BParse a b [c]
upTo 0 _ = return []
upTo n f = or (f >>= return . return) $ do x <- f; xs <- upTo (n-1) f; return (x:xs)

{- Please do not uncomment this
not :: BParse a b c -> BParse a b c
not (BParse f) = BParse p where p c x = case f c x of
					Nothing -> c undefined x
					_ -> Nothing
-}

runBParse :: BParse a b b -> a -> Maybe (b,a)
runBParse p = bparse p (\a b -> Just (a,b))
