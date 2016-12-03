(add-to-list 'load-path "~/.emacs.d/lisp")

(setq-default indent-tabs-mode nil)
(global-set-key (kbd "<f1>") 'goto-line)

(setq-default fill-column 80)
(add-hook 'text-mode-hook 'turn-on-auto-fill)

;; elpa, melpa & marmalade walk into a bar
(require 'package)
(setq package-archives '(("gnu" . "http://elpa.gnu.org/packages/")
                         ("marmalade" . "http://marmalade-repo.org/packages/")
                         ("melpa" . "https://melpa.org/packages/")))
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

(require 'clojure-mode)

;;
;; replace (fn with (λ to make self feel geeky
;;
(eval-after-load 'clojure-mode
  '(font-lock-add-keywords
    'clojure-mode `(("(\\(fn\\>\\)"
                     (0 (progn (compose-region (match-beginning 1)
                                               (match-end 1) "λ")
                               nil))))))

(setq-default indent-tabs-mode nil)
(setq cider-repl-display-help-banner nil)

;; kibit support
(require 'compile)
(add-to-list 'compilation-error-regexp-alist-alist
         '(kibit "At \\([^:]+\\):\\([[:digit:]]+\\):" 1 2 nil 0))
(add-to-list 'compilation-error-regexp-alist 'kibit)


(setq tramp-default-method "ssh")
(require 'tls)
(setq tls-program '("openssl s_client -connect %h:%p -no_ssl2 -ign_eof"))
(require 'erc)
(erc-autojoin-mode t)
(setq erc-autojoin-channels-alist)

;; avoid annoying slow autosaves when using tramp
(setq tramp-auto-save-directory "/tmp")

(setq-default indent-tabs-mode nil)
(setq clojure-defun-style-default-indent t)
(setq clojure-indent-style :always-align)
(eval-after-load "clojure-mode"
   '(progn
      (define-clojure-indent
        (:require 0)
        (:import 0))))



(autoload 'visual-basic-mode "visual-basic-mode" "Visual Basic mode." t)
(setq auto-mode-alist (append '(("\\.\\(frm\\|bas\\|asp\\|cls\\)$" .
                                 visual-basic-mode)) auto-mode-alist))



;; autocomplete via TAB
(setq company-idle-delay nil) ; dont autocomplete always
(global-set-key (kbd "M-TAB") #'company-indent-or-complete-common) ;; triggered via TAB
(add-hook 'cider-repl-mode-hook #'company-mode) ;; in cider-repl
(add-hook 'cider-mode-hook #'company-mode) ;; in cider

(ido-mode 1)
(setq ido-separator "\n")
