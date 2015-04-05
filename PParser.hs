-- Parrallel parsing - an experiment by AtnNn
-- Copyright (C) 2006-2007 Etienne Laurin
-- 
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the ATN Universal Public License as published by
-- the Etienne Laurin.
-- 
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- ATN Universal Public License for more details.
-- 
-- You should have received a copy of the ATN Universal Public License along
-- with this program; if not, write to Etienne Laurin <atnnn@atnnn.com>.

module PParser where

import Data.Char

data PType e s a = PFun (s -> e -> (s, PParser e s a))
		 | PNop (s -> (s, PParser e s a))
		 | PRet (s -> (s, a))
		 | PErr (s -> (s, String))

data PParser e s a = PParser [PType e s a]

type PPState e s a = [(s, PType e s a)]

instance Monad (PParser e s) where
    return a = PParser [PRet $ \s -> (s, a)]
    (PParser l) >>= g =
	PParser $ flip map l $ \p -> case p of
		  PFun f -> PFun $ \s e -> case f s e of (s, p) -> (s, p >>= g)
		  PRet f -> PNop $ \s -> case f s of (s, a) -> (s, g a)
		  PErr f -> PErr f
		  PNop f -> PNop $ \s -> case f s of (s, p) -> (s, p >>= g)
    fail err = PParser [PErr $ \s -> (s, err)]

showState :: (Show s, Show a) => (s, PType e s a) -> String
showState (s, PFun _) = "[" ++ show s ++ "] Unfinished"
showState (s, PNop _) = "[" ++ show s ++ "] Unevaluated"
showState (s, PErr f) = case f s of (s,err) -> "[" ++ show s ++ "] Error: " ++ err
showState (s, PRet f) = case f s of (s,a) -> "[" ++ show s ++ "] Value: " ++ show a

showStates :: (Show s, Show a) => PPState e s a -> String
showStates =  unlines . map showState

prepare :: PParser e s a -> s -> PPState e s a
prepare (PParser l) s = map ((,) s) l

pparse1 :: e -> PPState e s a -> PPState e s a
pparse1 e = concatMap (uncurry (pparse1t e))

pparse1t :: e -> s -> PType e s a -> PPState e s a
pparse1t e s (PFun f) = case f s e of (s, p) -> prepare p s
pparse1t _ s (PRet f) = case f s of (s, _) -> prepare (fail "continuing after result found") s
pparse1t _ s (PErr f) = case f s of (s, err) -> prepare (fail err) s
pparse1t e s (PNop f) = case f s of (s, p) -> pparse1 e (prepare p s)

results :: PPState e s a -> [Either String a]
results ((s,PRet f):xs) = (Right $ snd (f s)) : results xs
results ((s,PErr f):xs) = (Left $ snd (f s)) : results xs
results (_:xs) = results xs
results [] = []

clean :: PPState e s a -> PPState e s a
clean = concatMap clean1

strict :: PPState e s a -> PPState e s a
strict = concatMap strict1

clean1 :: (s, PType e s a) -> PPState e s a
clean1 (s, PErr _) = []
clean1 a = [a]

strict1 :: (s, PType e s a) -> PPState e s a
strict1 (s, PNop f) = case f s of (s,p) -> strict $ prepare p s
strict1 (s, PErr f) = case f s of (s,err) -> [(s,PErr $ flip (,) err)] -- is this necessary? doees it work?
strict1 (s, PRet f) = case f s of (s,a) -> [(s,PRet $ flip (,) a)]   -- same here?
strict1 a = [a]

pparse :: [e] -> PPState e s a -> PPState e s a
pparse [] p = strict p
pparse (x:xs) p = pparse xs (clean $ pparse1 x p)

pparseTrace :: [e] -> PPState e s a -> [PPState e s a]
pparseTrace = pparseTrace' []
    where pparseTrace' l [] p = reverse (p:l)
	  pparseTrace' l (x:xs) p = pparseTrace' (p:l) xs $ strict $ pparse1 x $ clean p

item :: PParser e s e
item = PParser [PFun $ \s e -> (s, return e)]

get :: PParser e s s
get = PParser [PRet $ \s -> (s, s)]

set :: s -> PParser e s ()
set s = PParser [PRet $ \_ -> (s,())]

modify :: (s -> s) -> PParser e s ()
modify f = get >>= set . f

(&&&) :: PParser e s a -> PParser e s b -> PParser e s (a,b)
f &&& g = do a <- f; b <- g; return (a,b)

(|||) :: PParser e s a -> PParser e s a -> PParser e s a
(PParser l) ||| (PParser m) = PParser (l ++ m)

many :: PParser e s a -> PParser e s [a]
many p = (do x <- p; return [x]) ||| (do x <- p; xs <- many p; return (x:xs))

single :: (e -> Bool) -> PParser e s e
single p = do x <- item; if p x then return x else fail "could not match single"

range :: Ord e => e -> e -> PParser e s e
range a b = do x <- item; if a <= x && x <= b then return x else fail "could not match range"

char :: Eq e => e -> PParser e s e
char c = single (==c) !!! const "could not match char"

string :: Eq e => [e] -> PParser e s [e]
string s = (sequence $ map char s) !!! const "could not match string"

digit :: PParser Char s Char
digit = single isDigit !!! const "could not match digit"

(!!!) :: PParser e s a -> (s -> String) -> PParser e s a
(PParser l) !!! err = PParser $ map changeError l
    where changeError (PFun f) = PFun $ \s e -> case f s e of (s, p) -> (s, p !!! err)
	  changeError (PRet f) = PRet f
	  changeError (PErr f) = PErr $ \s -> case f s of (s,_) -> (s, err s)
	  changeError (PNop f) = PNop $ \s -> case f s of (s,p) -> (s, p !!! err)

test p v s = putStr $ unlines $ map showStates $ pparseTrace v $ prepare p s 

testParser = lengthP ||| number

lengthP = do many (modify (+1) >> item)
	     get
number = do n <- many digit
	    return $ read n
