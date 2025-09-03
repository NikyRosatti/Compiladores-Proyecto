--Programa principal
⟨program⟩ → program { ⟨code⟩ }

--Cuerpo del programa
⟨code⟩ → ⟨var decl⟩ code | ⟨method decl⟩ code | ε 

--Declaraciones de variables 
⟨var_decl⟩ → ⟨type⟩ ⟨id⟩ = ⟨expr⟩ ;

--Declaraciones de métodos
⟨method_decl⟩ → ⟨all_types⟩ ⟨id⟩ ( ⟨params⟩ ) ⟨block⟩
            |  ⟨all_types⟩ ⟨id⟩ ( ⟨params⟩ ) extern ;

⟨all_types⟩ → ⟨type⟩ | void

⟨params⟩ → ⟨type⟩ ⟨id⟩ , params | ⟨type⟩ ⟨id⟩ | ε 

--Bloques de código
⟨block⟩ → {⟨block_decl⟩ ⟨block_statement⟩ }

⟨block_decl⟩ → ⟨var decl⟩ ⟨block_decl⟩ | ε

⟨block_statement⟩ → ⟨statement⟩ ⟨block_statement⟩ | ε

⟨type⟩ → integer | bool

--Sentencias
⟨statement⟩ → ⟨id⟩ = ⟨expr⟩ ;
            | ⟨method call⟩ ;
            | if ( ⟨expr⟩ ) then ⟨block⟩
            | if ( ⟨expr⟩ ) then ⟨block⟩ else ⟨block⟩
            | while ⟨expr⟩ ⟨block⟩
            | return ;
            | return ⟨expr⟩ ;
            | ⟨block⟩
            | ;

--Llamadas a métodos
⟨method call⟩ → ⟨id⟩ ( ⟨args⟩ )

⟨args⟩ → ⟨expr⟩ , ⟨args⟩ | ⟨expr⟩ | ε

--Expresiones
⟨expr⟩ → ⟨id⟩
        | ⟨method call⟩
        | ⟨literal⟩
        | ⟨expr⟩ ⟨bin op⟩ ⟨expr⟩
        | - ⟨expr⟩
        | ! ⟨expr⟩
        | ( ⟨expr⟩ )

--Operadores
⟨bin op⟩ → ⟨arith op⟩ | ⟨rel op⟩ | ⟨cond op⟩

⟨arith op⟩ → + | - | * | / | %

⟨rel op⟩ → < | > | ==

⟨cond op⟩ → && | ||

--Identificadores y literales
⟨literal⟩ → ⟨integer_literal⟩ | ⟨bool_literal⟩

⟨id⟩ → ⟨alpha⟩ ⟨id_body⟩

⟨id_body⟩ → ⟨alpha_num⟩ ⟨id_body⟩ | ε

⟨alpha_num⟩ → ⟨alpha⟩ | ⟨digit⟩ | _

⟨alpha⟩ → a | b | ... | z | A | B | ... | Z

⟨digit⟩ → 0 | 1 | 2 | ... | 9

⟨integer_literal⟩ → ⟨digit⟩ ⟨integer_literal⟩ | ⟨digit⟩  

⟨bool_literal⟩ → true | false



