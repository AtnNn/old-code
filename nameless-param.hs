{-# LANGUAGE UnicodeSyntax, OverloadedStrings #-}

import Data.Aeson
import Data.Aeson.Types
import Data.Text (Text)

person ∷ String → Float → Bool → Value
person name height over18 = object [
  "name" .= name,
  "height" .= height,
  "over18" .= over18]

infixr 3 $-
--($-) ∷ (d → a → d) → (d → k) → d → a → k
($-) ∷ (d → a → e) → (e → k) → d → a → k
($-) f g d a = g $ f d a

field ∷ ToJSON a ⇒ Text → [Pair] → a → [Pair]
field k d v =  (k, toJSON v) : d

fieldf ∷ Text → (a → Value) → [Pair] → a → [Pair]
fieldf k f d v =  (k, f v) : d

optfield ∷ ToJSON a ⇒ Text → [Pair] → Maybe a → [Pair]
optfield k d (Just v) = field k d v
optfield _ d Nothing = d

person' ∷ String → Maybe Float → Bool → Value
person' = ($[])
  $  field "name"
  $- optfield "height"
  $- field "over18"
  $- object

test ∷ String → Int → String → String → Int → [String]
test = ($"")
  $  (++)
  $- (. show) . (++)
  $- (return .) . (++)
  $- flip (:)
  $- (. take) . flip map
       
results ∷ String → Float → Bool → String → Value
results = ($ person)
  $  ($)
  $- ($)
  $- (. ($))
     ((return . ("person".=)) .) 
  $- field "diagnosis"
  $- object
  
data Foo = Foo Int Int Int