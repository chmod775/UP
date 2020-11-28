class num {
  constructor() {
    this.val = 0;
  }

  Inc(step) {
    this.val = this.val + step;
  }

  Less(ref) {
    return this.val < ref;
  }

  ToString() {
    return this.val.toString();
  }
}

function test(cnt) {
  if (cnt < 100)
    test(cnt + 1);
}


var i = new num();
var t = 0;
var a = "world";
var d = "Hello";

while (t < 80000) {
  t++;
  test(0);
  a = a + d + " " + t;
}

console.log(a);
