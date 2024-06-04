# FastChess
Faster and Bug Free Chess engine in C with python api using ctype

FastChess is a high-performance Python chess engine library built using ctypes, providing significant speed improvements over traditional Python chess libraries like python-chess. This library is designed for speed and efficiency, making it suitable for use in applications requiring rapid chess move generation and analysis.

## Features

- **High Performance**: FastChess leverages ctypes to interface with a C chess backend, resulting in significantly faster move generation and analysis compared to pure Python implementations.
  
- **Ease of Use**: The library provides a simple and intuitive API for performing common chess operations such as generating legal moves.

- **Modular Design**: FastChess is designed with modularity in mind, allowing users to easily extend and customize its functionality to suit their specific needs.

## Installation

To install FastChess, simply clone the repository and run the setup script:

```bash
git clone https://github.com/thefcraft/FastChess.git
cd FastChess
python setup.py install
```

## Usage

Here's a basic example of how to use FastChess to generate legal moves for a given position:

```python
from FastChess import Board

# Create a new Board instance
board = Board(fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")

# Generate legal moves for the current position
legal_moves = board.generate_moves()

# Print the legal moves
for move in legal_moves:
    print(move)
```

## Contributing

Contributions to FastChess are welcome! If you encounter any bugs, have feature requests, or would like to contribute code, please open an issue or submit a pull request on GitHub.

## License

FastChess is licensed under the [MIT License](https://github.com/git/git-scm.com/blob/main/MIT-LICENSE.txt).
