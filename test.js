class Hello {
  constructor(name) {
    this.name = name;
  }
  Say() {
    console.log('Ciao ' + this.name + '!');
  }
}

var cnt = 0;
var str = '';
for (var a = 0; a < 100000; a++) {
  var o = new Hello(a.toString());
  cnt += 1.1;
  str = str + a.toString();
  o.Say();
}
console.log(cnt);
console.log(str);