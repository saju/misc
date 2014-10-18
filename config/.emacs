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
(require 'clojure-mode)
(add-hook 'cider-mode-hook 'cider-turn-on-eldoc-mode)
(setq nrepl-hide-special-buffers t)
(setq cider-prefer-local-resources t)
(setq cider-repl-result-prefix "=> ")
(setq cider-repl-history-file "~/.emacs.d/clojure-repl-history")
