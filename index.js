const binding = require('./build/Release/uct');

const mcts = {
    uct: binding.uct,
    simulation: binding.simulation
};

module.exports = mcts;
