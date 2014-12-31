(global-set-key (kbd "<f1>") 'goto-line)

;; elpa, melpa & marmalade walk into a bar
(require 'package)
(setq package-archives '(("gnu" . "http://elpa.gnu.org/packages/")
                         ("marmalade" . "http://marmalade-repo.org/packages/")
                         ("melpa" . "http://melpa.milkbox.net/packages/")))
(package-initialize)

;; GUI Emacs needs to pickup same ENV as the shell
(when (memq window-system '(mac ns))
  (exec-path-from-shell-initialize))


;; setup paren-mode to hilight expressions
(setq show-paren-delay 0)           
(show-paren-mode t)                 
(setq show-paren-style 'expression)

;; playing with random color themes 
(require 'color-theme)
(color-theme-initialize)
(defun set-color-theme() 
  (interactive)
  (setq current-theme (car (nth (random (length color-themes)) color-themes)))
  (print current-theme)
  (funcall current-theme))

(set-color-theme)
(global-set-key (kbd "<f7>") 'set-color-theme)

;; clojure stuff
;; cider
(require 'clojure-mode)
(add-hook 'cider-mode-hook 'cider-turn-on-eldoc-mode)
(setq nrepl-hide-special-buffers t)
(setq cider-prefer-local-resources t)
(setq cider-repl-result-prefix "=> ")
(setq cider-repl-history-file "~/.emacs.d/clojure-repl-history")

;;
;; replace (fn with (λ to make self feel geeky
;;
(eval-after-load 'clojure-mode
  '(font-lock-add-keywords
    'clojure-mode `(("(\\(fn\\>\\)"
                     (0 (progn (compose-region (match-beginning 1)
                                               (match-end 1) "λ")
                               nil))))))


;; dumb inferior-repl support as alternative to cider nREPL
;; "repl" is my shell script that wraps jline over clojure.jar, so i can use
;; a readline like repl on the terminal too
(global-set-key (kbd "<f2>") '(lambda ()
				(interactive)
				(add-hook 'clojure-mode-hook '(lambda ()
								(local-set-key (kbd "\C-x\C-e" 'lisp-eval-last-sexp))))
				(setq inferior-lisp-program "repl")))
					  
(global-set-key (kbd "<f3>") 'run-lisp)


(setq tramp-default-method "ssh")

;; kibit support
(require 'compile)
(add-to-list 'compilation-error-regexp-alist-alist
         '(kibit "At \\([^:]+\\):\\([[:digit:]]+\\):" 1 2 nil 0))
(add-to-list 'compilation-error-regexp-alist 'kibit)


(defun kibit ()
  "Run kibit on the current project.
Display the results in a hyperlinked *compilation* buffer."
  (interactive)
  (compile "lein kibit"))

(defun kibit-current-file ()
  "Run kibit on the current file.
Display the results in a hyperlinked *compilation* buffer."
  (interactive)
  (compile (concat "lein kibit " buffer-file-name)))


(require 'tls)
(setq tls-program '("openssl s_client -connect %h:%p -no_ssl2 -ign_eof"))
(require 'erc)
(erc-autojoin-mode t)
(setq erc-autojoin-channels-alist)

;; avoid annoying slow autosaves when using tramp
(setq tramp-auto-save-directory "/tmp")
