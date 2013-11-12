console = {}
console.log = function(a){
    print(a);
}

var avroparse = new AvroForGaia();

function avroparsefunc(a) {
    return JSON.stringify(avroparse(a));
}

console.log("Test : sari");
console.log(avroparsefunc("sari"));