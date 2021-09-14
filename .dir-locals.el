;; Some Emacs stuff:

;; A nice spell-check mode can be globally selected with:
;; mode is (add-hook 'prog-mode-hook 'flyspell-prog-mode)

;; Use American spell-checker for all the project.
((nil . ((ispell-local-dictionary . "american")
         ;; To set the mode from here instead of from .emacs:
         ;; (eval . (lambda () (flyspell-prog-mode)))
         )
))
