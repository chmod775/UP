# UPpercase

UPpercase (in short UP) is a programming language based on 3 simple concepts:
- Everithing is a class. Functions are Methods and Variables are Fields.
- Methods and Classes must start uppercase, Fields lowercase. (hence the name UPpercase)
- All operators are defined inside the classes using Methods. 

It's aimed to be embeddable, portable and extendable thanks to it's limited code size.
It is also not compiled to native code but instead pre-compiled in internal memory structures (no bytecode) and then interpreted.

UPpercase get features and inspiration from the trusted C, C# and BASIC all combined.

## Contents

- [Getting started](#getting-started)
- [Documentation](#documentation)
  - [Class](#class)
  - Field
  - Method
    - Operators

## Getting started

## Documentation
UP use a basic concept, "Everithing is a class". Starting from the Program which is an "Hidden" Class that implement the method Main.

Here it is a simple HelloWorld.up file as reference for the structure.

```javascript
// Program { // Program class definition can be omitted.
  
  welcomeContent : String = "Hello"; // Field definition. They must have a type and an initialization value.

  GenerateWelcome(recipientContent : String = "World") : String { // Method definition with one optional Argument and String return.
    return = this.welcomeContent + " " + recipientContent;
  }

  Main() { // Main Method definition.
    GenerateWelcome();
  }

// }
```

### Class
Class names MUST start Uppercase and have at least 1 constructor.
Class is a statement that can contain:
  - Other class definitions
  - Methods / Constructors (methods without name)
  - Fields

```javascript
Class {
  () { } // Constructor definition

  field : String = "";

  Method() { }
}

instance : Class = Class();
```

In case of nested classes, they will be available from the outside using the access operator (.).
This is very usefull for creating "groups".

```javascript
Engine {
  () { } // Constructor definition

  cc : Number = 0;

  KmBased {
    kmDone : Number = 0;
  }

  HoursBased {
    hrDone : Number = 0;
  }
}

carEngine : Engine.KmBased = Engine.KmBased();
boatEngine : Engine.HoursBased = Engine.HoursBased();
```

They can also inherit other classes (using __Children : Parent__) or the container class as parent (using __Children ::__).

```javascript
ParentClass {
  AnotherChildrenClass :: {

  }
}

ChildrenClass : ParentClass {

}
```

## Usage

```bash

```

## Contributing

I'll be very thankfull if you can buy me a beer or a coffee using the Donate button.

Thanks for using UPpercase.

## License
[MIT](https://choosealicense.com/licenses/mit/)