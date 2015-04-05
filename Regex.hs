-- Regex library implemented using BParse.hs
-- written by Etienne Laurin

-- Unfinished but already fairly complete, see examples at end of file


module Regex where

import Prelude hiding (maybe,or,and)
import BParse
import Data.Maybe (isJust, isNothing)
import Data.Char

data RegexOptions = RegexOptions { caseSensitive :: Bool, beginning :: Bool}

match :: String -> String -> Bool
match re string = isJust $ runBParse (regex re) string

regex :: String -> BParse String b String
regex re = case runBParse parse re of
	   Just (a,"") -> a
	   _ -> error "invalid regular expression"
    where parse = do char '/'
		     exp <- expression
		     char '/'
		     opt <- options
		     return $ exp opt `or` (few get >> exp (opt { beginning = False }))

options :: BParse String b RegexOptions
options = do c <- maybe $ char 'i' -- only i for now
	     return $ RegexOptions { caseSensitive = isNothing c, beginning = True}

expression :: BParse String b  (RegexOptions -> BParse String c String)
expression = do exp <- list (many $ lhs `and` maybe rhs) (char '|')
		return $ \o -> oneOf $ map (run o) exp
		-- exp :: [[(Opt -> Parser [String], Maybe (Parser [String] -> Parser [String]))]]
    where joinRL o (p, Just f) = f $ p o
	  joinRL o (p, Nothing) = p o
	  order = foldl (\a b -> do (a,b) <- and a b; return (a++b)) (return "")
	  run o e = order $ joinRL o (head e) : map (joinRL (o { beginning = False })) (tail e)

lhs :: BParse String b (RegexOptions -> BParse String c String)
lhs = oneOf [ char '.' >> return (const $ do c <- get; return [c])
	    , char '^' >> return (\o -> if beginning o then return "" else nothing)
	    , char '$' >> return (\o -> do x <- maybe peek; if isJust x then nothing else return "")
	    , parens
	    , squares
	    , escape
	    , literal
	    ]

rhs = oneOf [ char '*' >> return (\p -> do l <- or (many p) (return []); return $ concat l) -- *? +? ??
	    , char '+' >> return (\p -> many p >>= (return . concat))
	    , char '?' >> return (flip or $ return "")
	    , occs
	    ]

-- emacs seems to match ()[]{} inside quotes and comments and it messes up the automatic layout

parens = do char '(' -- )
	    e <- expression
	    char {- -} ')'
	    return e

squares = do char '[' -- ]
	     rev <- maybe $ char '^'
	     l <- many sq_elem
	     char {- -} ']'
	     let adj = if isJust rev then (not .) else id in
		 return $ \o -> oneOf $ map (string1 . adj . ($ o)) l

sq_elem :: BParse String b (RegexOptions -> Char -> Bool)
sq_elem = oneOf [ sq_escape
		, sq_range
		, sq_single
		]

sq_escape = char '\\' >> (oneOf $ [ integer >>= (return . const . (==) . chr)
				  ] ++ map check ('-':specialChars))
    where check c = char c >> (return $ const (==c))

sq_range = do a <- sq_lit
	      char '-'
	      b <- sq_lit
	      return $ \o -> case caseSensitive o of
			     True -> \c -> (a<=c)&&(c<=b)
			     False -> \c -> the min a <= the max c && the min c >= the max b
    where the f a = f (toUpper a) (toLower a)
			  

sq_single = do c <- sq_lit
	       return $ \o -> if caseSensitive o
			        then (==c)
			        else \a -> (a == toUpper c) || (a == toLower c)

sq_lit = single lit where lit c = c /= '/' && c /= ']'

escape = char '\\' >> (oneOf $ [ integer >>= (return . const . string . (:[]) . chr)
			       ] ++ map check specialChars)
    where check c = char c >> (return $ const (string [c]))

specialChars = ['\\', '/', '|', ')', '(', '[', ']', '{']

literal = do c <- single $ not . flip elem specialChars
	     return $ \o -> (if caseSensitive o then char c else caseless c) >>= return . return

occs = do char '{' -- }5A
	  n1 <- integer
	  mn2 <- maybe (char ',' >> integer)
	  char {- -} '}'
	  let n2 = case mn2 of Nothing -> 0; Just n2 -> (n1-n2) in
	      return $ \p -> do a <- exactly n1 p; b <- upTo n2 p; return $ concat $ a++b

string1 = exactly 1 . single


{- current status:

*Regex> match "/test/" "tes"
False
*Regex> match "/test/" "test"
True
*Regex> match "/test/i" "Test"
True
*Regex> match "/test/" "Test"
False
*Regex> match "/[Tt]est/" "Test"
True
*Regex> match "/[Tt]est/" "pest"
False
*Regex> match "/a*b{3}/" "aaabbb"
True
*Regex> match "/a*b{3}/" "aaabb"
False
*Regex> match "/a*b{3}/" "bb"
False
*Regex> match "/a*b{3}/" "bbb"
True
*Regex> match "/a+b{3}/" "bbb"
False
*Regex> match "/a+b{3}/" "abbb"
True
*Regex> match "/a+b{3}/" "acbbb"
False
*Regex> match "/foo|(bl(ah|eh))|bar/" "foo"
True
*Regex> match "/foo|(bl(ah|eh))|bar/" "fo"
False
*Regex> match "/foo|(bl(ah|eh))|bar/" "blah"
True
*Regex> match "/foo|(bl(ah|eh))|bar/" "blahf"
True
*Regex> match "/foo|(bl(ah|eh))|bar/" "bleh"
True
*Regex> match "/foo|(bl(ah|eh))|bar/" "blue"
False
*Regex> match "/foo|(bl(ah|eh))|bar/" "bar"
True

-}
