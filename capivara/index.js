
bfWriteFile("../test2.txt","testando 123",() => {
    bfReadFile("../test2.txt",(data) => {
        print(`Conteudo do arquivo: ${data}`);
    })
})