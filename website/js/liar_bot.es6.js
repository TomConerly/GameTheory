var derpdata = require("./derp.json");

class LiarBot {
    
  constructor(options = {}) {
    this.name = options.name || 'no name';
  }

  getName() {
    return this.name;
  }

  toString() {
    return `Liar - ${this.getName()}`;
  }
}


module.exports = LiarBot;
