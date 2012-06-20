# ------------------------------------------------------------------------------
# MADlib Design Document
# ------------------------------------------------------------------------------

# FIXME: The following works with TeX Live but is probably not portable
set(LATEX_COMPILER_FLAGS "-interaction=nonstopmode -shell-escape"
    CACHE STRING "Flags passed to latex")
set(LATEX_USE_SYNCTEX TRUE CACHE STRING "")

include(UseLATEX)

add_latex_document(design.tex
    BIBFILES ../literature.bib
    DEFAULT_PDF
    MANGLE_TARGET_NAMES
)
