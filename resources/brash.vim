" Place in ~/.config/nvim/syntax/
" Used as reference - https://vimdoc.sourceforge.net/htmldoc/syntax.html#syntax


" Constant
syn match brashBool '\<true\>'
syn match brashBool '\<false\>'
syn match brashString '"[^"]*"'
syn match brashCharLiteral "'.'"
syn match brashNumLiteral '\<\d\.\?\d*\>'

hi def link brashBool Boolean
hi def link brashString String
hi def link brashCharLiteral String
hi def link brashNumLiteral Float


" Function
syn match brashFunction '\<func\>'
hi def link brashFunction Function


" Statements
syn match brashControlflow '\<if\>'
syn match brashControlflow '\<else\>'
syn match brashLoops       '\<while\>'
syn match brashKeywords    '\<print\>'
syn match brashKeywords    '\<var\>'
syn match brashKeywords    '\<return\>'
syn match brashOperators   "[=\&\|\^\+\-\*/%><]"

hi def link brashControlflow Conditional
hi def link brashLoops Repeat
hi def link brashOperators Operator
hi def link brashKeywords Keyword


" Type
syn match brashType '\<String\>'
syn match brashType '\<Number\>'
syn match brashType '\<Boolean\>'
syn match brashType '\<Bool\>'

hi def link brashType Type


" Comments
syn match brashComment '//.*$'
syn match brashComment '#.*$'
hi def link brashComment Comment
