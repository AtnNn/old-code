;; eager pattern matching on s-expressions with continuations
;; Copyright (C) 2007 Etienne Laurin
;;
;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the ATN Universal Public License as published by
;; the Etienne Laurin.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; ATN Universal Public License for more details.
;;
;; You should have received a copy of the ATN Universal Public License along
;; with this program; if not, write to Etienne Laurin <atnnn@atnnn.com>.

(define (match pat exp)
  (cond ((pair? pat)
         (case (car pat)
           ((quote) (if (equal? (cadr pat) exp) '() #f))
           ((...)
            (match-skip (cdr pat) exp #f #f))
           (else
            (if (eq? (car pat) 'unquote)
                (call/cc
                 (lambda (k) (list (list (cadr pat) exp (lambda () (k #f))))))
                (if (and (pair? (car pat)) (eq? 'unquote-splicing (caar pat)))
                    (match-skip (cdr pat) exp (cadar pat) '())
                    (and (pair? exp)
                         (let ((head (match (car pat) (car exp))))
                           (and head (join-match head (match (cdr pat) (cdr exp)))))))))))
        ((eq? '* pat) '())
        ((equal? pat exp) '())
        (#t #f)))

(define (match-skip pat exp var rval)
  (if (null? pat)
      (if var 
          (call/cc
           (lambda (k) (list (list var exp (lambda () (k #f))))))
          '())
      (let ((a (match pat exp)))
        (or (and a
                 (if var
                     (call/cc
                      (lambda (k)
                        (join-match (list (list var (reverse rval) (lambda () (k #f)))) a)))
                     a))
            (and (pair? exp) (match-skip pat (cdr exp) var (if rval (cons (car exp) rval) #f)))))))

(define (join-match a b)
  (and a b (append a b)))

(define *retry-continuations* '())

(define (push-continuation! k)
  (set! *retry-continuations* (cons k *retry-continuations*)))

(define (pop-continuation!)
  (set! *retry-continuations* (cdr *retry-continuations*)))

(define (get-continuation)
  (car *retry-continuations*))

(define (get-match pat exp)
  (let* ((first? #t)
         (r (match pat exp)))
    (if first?
        (begin (set! first? #f) r)
        ((get-continuation) r))))

(define (retry-match retry)
  (retry))

(define (get-retry-match retry)
  (call/cc
   (lambda (k)
     (push-continuation! k)
     (let ((r (retry)))
       (pop-continuation!)
       r))))

(define (last l)
  (if (null? (cdr l)) (car l) (last (cdr l))))

(define (next-match match)
  (if (or (null? match) (boolean? match)) #f
      (retry-match (caddr (last match)))))

(define (get-next-match match)
  (if (or (null? match) (boolean? match)) #f
      (get-retry-match (caddr (last match)))))

(define (match-all pat exp)
  (let ((a (get-match pat exp)))
    (if (not a) '()
        (let loop ((r (list a)))
          (let ((a (get-next-match (car r))))
            (if a (loop (cons a r))
                (reverse r)))))))

(define (match-guard pat ok? exp)
  (let ((a (match pat exp)))
    (and a (or (and (apply ok? (map cadr a)) a)
               (next-match a)))))

(define (get-match-guard pat ok? exp)
  (let* ((first? #t)
         (r (match-guard pat ok? exp)))
    (if first?
        (begin (set! first? #f) r)
        ((get-continuation) r))))

(define (replace-all pat fun exp rec?)
  (let loop ((exp exp))
    (let ((a (match pat exp)))
      (cond (a (let ((r (apply fun (map cadr a))))
                 (if rec? (loop r) r)))
            ((pair? exp) (cons (loop (car exp))
                               (loop (cdr exp))))
            (#t exp)))))

(define (replace-all-guard pat ok? fun exp rec?)
  (let loop ((exp exp))
    (let ((a (match-guard pat ok? exp)))
      (cond (a (let ((r (apply fun (map cadr a))))
                 (if rec? (loop r) r)))
            ((pair? exp) (cons (loop (car exp))
                               (loop (cdr exp))))
            (#t exp)))))

(define (match-vars pat)
  (let loop ((pat pat))
    (if (pair? pat)
        (case (car pat)
          ((quote) '())
          ((unquote unquote-splicing)
           (list (cadr pat)))
          (else (append (loop (car pat)) (loop (cdr pat)))))
        '())))



(define-macro (match-lambda . l)
  (let ((args (gensym 'args)) (m (gensym 'm)))
    `(lambda ,args
       ,(let loop ((l l))
          (if (null? l) '(error "incomplete patterns")
              (let ((pat (caar l)) (body (cdar l)))
                `(let ((,m (match ',pat ,args)))
                   (if ,m (apply (lambda ,(match-vars pat) . ,body) (map cadr ,m))
                       ,(loop (cdr l))))))))))

(define-macro (match-guard-lambda . l)
  (let ((args (gensym 'args)) (m (gensym 'm)))
    `(lambda ,args
       ,(let loop ((l l))
          (if (null? l) '(error "incomplete patterns")
              (let* ((pat (caar l)) (ok? (cadar l)) (body (cddar l)) (vars (match-vars pat)))
                `(let ((,m (match-guard ',pat (lambda ,vars ,ok?) ,args)))
                   (if ,m (apply (lambda ,vars . ,body) (map cadr ,m))
                       ,(loop (cdr l))))))))))
