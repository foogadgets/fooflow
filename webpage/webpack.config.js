const path = require('path');
module.exports = {
  entry: './src/chart.js',
  output: {
    filename: 'chart.js',
    path: path.resolve(__dirname, 'dist'),
  },
  mode: 'production',
};
