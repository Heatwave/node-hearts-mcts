const binding = require('./build/Release/uct');

const mcts = {
    uct: binding.uct,
    simulation: binding.simulation,
    simulate: binding.simulate
};

module.exports = mcts;
