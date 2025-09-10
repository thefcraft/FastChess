from fastchess import Board
fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
fastboard = Board(fen=fen)
print(fastboard)
print(f"[{', '.join(sorted([repr(move) for move in fastboard.legal_moves()]))}]")