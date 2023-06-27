
bfWriteFile("../test2.txt","testando '123",() => {
    bfReadFile("../test2.txt",(err,data) => {
        print(data);
    })
})