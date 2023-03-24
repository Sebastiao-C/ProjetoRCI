# ProjetoRCI

A interface de utilizador consiste nos seguintes comandos, os quais devem ser abreviados
pelas suas letras iniciais.
• new
Criação de um anel contendo apenas o nó.

• bentry boot boot.IP boot.port
Entrada do nó no anel ao qual pertence o nó boot com endereço IP boot.IP e
porto boot.port. Embora apareça primeiro nesta lista, este será o último
comando a ser implementado.

• pentry pred pred.IP pred.port
Entrada do nó no anel sabendo que o seu predecessor será o nó pred com
endereço IP pred.IP e porto pred.port. Com este comando, um nó pode entrar
no anel sem que a funcionalidade de procura de chaves esteja ativa.

• chord i i.IP i.port
Criação de um atalho para o nó i com endereço IP i.IP e porto i.port.

• echord
Eliminação do atalho.

• show
Mostra o estado do nó, consistindo em: (i) a sua chave, endereço IP e porto; (ii) a
chave, endereço IP e porto do seu sucessor; (iii) a chave, endereço IP e porto do
seu predecessor; e, por fim, (iv) a chave, endereço IP e porto do seu atalho, se
houver.

• find k
Procura da chave k, retornando a chave, o endereço IP e o porto do nó à qual a
chave pertence.

• leave
Saída do nó do anel.

• exit
Fecho da aplicação.

