from FastChess import Board, Move, Piece, SQUARES, pos_decoder
if __name__ == '__main__':
    # fen =  '8/8/1b4p1/3q1k1p/1Pp1ppP1/2P4P/4QP2/2B2K2 b - g3 0 45'
    # fen = 'rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 1 2'
    fen = None
    # board = Board(fen)
    # print(board.is_attackers(4, False))
    # print(board.is_attackers(4, True))
    # board.push(Move.from_uci('e2e4Q'))
    # print(board.minmax(3, True))
    # print(board.alphabeta(4, True))
    # print(board.is_attackers(11, True))
    # moves = list(board.legal_moves())
    # print(' '.join(([repr(move) for move in board.legal_moves()])))
    # board.push(move)
    # print(len(list(board.legal_moves())))
    # print(board.minmax(3, True))
    # quit()
    
    # testing
    import tkinter as tk
    import cairosvg
    import time
    from PIL import Image, ImageTk
    from io import BytesIO
    
    import random

    WHITE_PAWN_PREFERRED_COORDINATES = [
                0,  0,  0,  0,  0,  0,  0,  0,
                90, 90, 90, 90, 90, 90, 90, 90,
                30, 30, 40, 60, 60, 40, 30, 30,
                10, 10, 20, 40, 40, 20, 10, 10,
                5,  5, 10, 20, 20, 10,  5,  5,
                0,  0,  0,-10,-10,  0,  0,  0,
                5, -5,-10,  0,  0,-10, -5,  5,
                0,  0,  0,  0,  0,  0,  0,  0
    ]
    BLACK_PAWN_PREFERRED_COORDINATES = [
                0,  0,  0,  0,  0,  0,  0,  0,
                5, -5,-10,  0,  0,-10, -5,  5,
                0,  0,  0,-10,-10,  0,  0,  0,
                5,  5, 10, 20, 20, 10,  5,  5,
                10, 10, 20, 40, 40, 20, 10, 10,
                30, 30, 40, 60, 60, 40, 30, 30,
                90, 90, 90, 90, 90, 90, 90, 90,
                0,  0,  0,  0,  0,  0,  0,  0
    ]
    WHITE_KNIGHT_PREFERRED_COORDINATES = [
                -50,-40,-30,-30,-30,-30,-40,-50,
                -40,-20,  0,  5,  5,  0,-20,-40,
                -30,  5, 10, 15, 15, 10,  5,-30,
                -30,  5, 15, 20, 20, 15,  5,-30,
                -30,  5, 15, 20, 20, 15,  5,-30,
                -30,  5, 10, 15, 15, 10,  5,-30,
                -40,-20,  0,  0,  0,  0,-20,-40,
                -50,-40,-30,-30,-30,-30,-40,-50
    ]
    BLACK_KNIGHT_PREFERRED_COORDINATES = [
                -50,-40,-30,-30,-30,-30,-40,-50,
                -40,-20,  0,  0,  0,  0,-20,-40,
                -30,  5, 10, 15, 15, 10,  5,-30,
                -30,  5, 15, 20, 20, 15,  5,-30,
                -30,  5, 15, 20, 20, 15,  5,-30,
                -30,  5, 10, 15, 15, 10,  5,-30,
                -40,-20,  0,  5,  5,  0,-20,-40,
                -50,-40,-30,-30,-30,-30,-40,-50,
    ]
    WHITE_BISHOP_PREFERRED_COORDINATES = [
                -20,-10,-10,-10,-10,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5, 10, 10,  5,  0,-10,
                -10,  5,  5, 10, 10,  5,  5,-10,
                -10,  0, 10, 15, 15, 10,  0,-10,
                -10, 10, 10, 10, 10, 10, 10,-10,
                -10,  5,  0,  0,  0,  0,  5,-10,
                -20,-10,-10,-10,-10,-10,-10,-20
    ]
    BLACK_BISHOP_PREFERRED_COORDINATES = [
                -20,-10,-10,-10,-10,-10,-10,-20,
                -10,  5,  0,  0,  0,  0,  5,-10,
                -10, 10, 10, 10, 10, 10, 10,-10,
                -10,  0, 10, 15, 15, 10,  0,-10,
                -10,  5, 10, 15, 15, 10,  5,-10,
                -10,  0, 10, 10, 10, 10,  0,-10,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -20,-10,-10,-10,-10,-10,-10,-20
    ]
    WHITE_ROOK_PREFERRED_COORDINATES = [
                0,  0,  0,  0,  0,  0,  0,  0,
                5, 20, 20, 20, 20, 20, 20,  5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                0,  0,  0,  5,  5,  0,  0,  0
    ]
    BLACK_ROOK_PREFERRED_COORDINATES = [
                0,  0,  0,  5,  5,  0,  0,  0,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                5, 20, 20, 20, 20, 20, 20,  5,
                0,  0,  0,  0,  0,  0,  0,  0,
    ]
    WHITE_QUEEN_PREFERRED_COORDINATES = [
                -20,-10,-10, -5, -5,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5,  5,  5,  5,  0,-10,
                -5,  0,  5, 10, 10,  5,  0, -5,
                -5,  0,  5, 10, 10,  5,  0, -5,
                -10,  0,  5,  5,  5,  5,  0,-10,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -20,-10,-10, -5, -5,-10,-10,-20
    ]
    BLACK_QUEEN_PREFERRED_COORDINATES = [
                -20,-10,-10, -5, -5,-10,-10,-20,
                -10,  0,  5,  0,  0,  0,  0,-10,
                -10,  5,  5,  5,  5,  5,  0,-10,
                -5,  0,  5, 10, 10,  5,  0, -5,
                -5,  0,  5, 10, 10,  5,  0, -5,
                -10,  0,  5,  5,  5,  5,  0,-10,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -20,-10,-10, -5, -5,-10,-10,-20
    ]
    WHITE_KING_PREFERRED_COORDINATES = [
                -50,-30,-30,-30,-30,-30,-30,-50,
                -30,-30,  0,  0,  0,  0,-30,-30,
                -30,-10, 20, 30, 30, 20,-10,-30,
                -30,-10, 30, 40, 40, 30,-10,-30,
                -30,-10, 30, 40, 40, 30,-10,-30,
                -30,-10, 20, 30, 30, 20,-10,-30,
                -30,-20,-10,  0,  0,-10,-20,-30,
                -50,-40,-30,-20,-20,-30,-40,-50
    ]
    BLACK_KING_PREFERRED_COORDINATES = [
                -50,-40,-30,-20,-20,-30,-40,-50,
                -30,-20,-10,  0,  0,-10,-20,-30,
                -30,-10, 20, 30, 30, 20,-10,-30,
                -30,-10, 30, 40, 40, 30,-10,-30,
                -30,-10, 30, 40, 40, 30,-10,-30,
                -30,-10, 20, 30, 30, 20,-10,-30,
                -30,-30,  0,  0,  0,  0,-30,-30,
                -50,-30,-30,-30,-30,-30,-30,-50
    ]
    def position_value(piece:str, square, noise:bool=True):
        square = pos_decoder(63-square)
        match piece:
            case b'p': return -BLACK_PAWN_PREFERRED_COORDINATES[square]+(random.random() if noise else 0)
            case b'P': return WHITE_PAWN_PREFERRED_COORDINATES[square]+(random.random() if noise else 0)
            case b'r': return -BLACK_ROOK_PREFERRED_COORDINATES[square]+(random.random() if noise else 0)
            case b'R': return WHITE_ROOK_PREFERRED_COORDINATES[square]+(random.random() if noise else 0)
            case b'b': return -BLACK_BISHOP_PREFERRED_COORDINATES[square]+(random.random() if noise else 0)
            case b'B': return WHITE_BISHOP_PREFERRED_COORDINATES[square]+(random.random() if noise else 0)
            case b'n': return -BLACK_KNIGHT_PREFERRED_COORDINATES[square]+(random.random() if noise else 0)
            case b'N': return WHITE_KNIGHT_PREFERRED_COORDINATES[square]+(random.random() if noise else 0)
            case b'q': return -BLACK_QUEEN_PREFERRED_COORDINATES[square]+(random.random() if noise else 0)
            case b'Q': return WHITE_QUEEN_PREFERRED_COORDINATES[square]+(random.random() if noise else 0)
            case b'k': return -BLACK_KING_PREFERRED_COORDINATES[square]+(random.random() if noise else 0)
            case b'K': return WHITE_KING_PREFERRED_COORDINATES[square]+(random.random() if noise else 0)
            case _: return 0
    def piece_value(piece:str):
        match piece:
            case b'p': return -100
            case b'P': return 100
            case b'r': return -500
            case b'R': return 500
            case b'b': return -300
            case b'B': return 300
            case b'n': return -300
            case b'N': return 300
            case b'q': return -900
            case b'Q': return 900
            case b'k': return -10000
            case b'K': return 10000
            case _: return 0
    def evaluate_board(board:Board):
        evaluation = 0
        if board.is_game_over():
            winner =  board.winner()
            if winner == True: evaluation += 10000
            elif winner == False: evaluation -= 10000
            return evaluation
        for square in SQUARES:
            piece = board.piece_at_raw(square)
            if piece == b'.': continue
            evaluation += piece_value(piece)
            evaluation += position_value(piece, square) 
        return evaluation
    def minmax(board:Board, depht, player:bool, return_move=True):
        # global nodes
        game_over = board.is_game_over()
        if return_move: 
            print("GAME OVER: ", game_over)
            if game_over: print("FEN: ", board.fen())
        if depht == 0 or game_over: return evaluate_board(board)
        bestmove = None
        if player:
            maxscore = -100000 #float('-inf')
            for move in board.legal_moves():
                tmp = board.copy()
                tmp.push(move) 
                # nodes+=1
                score = minmax(tmp, depht-1, False, return_move=False)
                if score > maxscore:
                    bestmove = move
                    maxscore = score
                del tmp
            if not return_move: return maxscore
        else:
            minscore = 100000 #float('inf')
            for move in board.legal_moves():
                tmp = board.copy()
                tmp.push(move)
                # nodes+=1
                score = minmax(tmp, depht-1, True, return_move=False)
                if score < minscore:
                    bestmove = move
                    minscore = score
                del tmp
            if not return_move: return minscore
        # print("nodes_per_sec: ", nodes/(time.time()-time_start))
        return bestmove
    def alphabeta(board:Board, depth, player:bool, alpha=-100000, beta=100000, return_move=True):
        # global nodes
        bestmove = None
        if depth == 0 or board.is_game_over(): return evaluate_board(board)
        if player:
            maxscore = -100000 #float('-inf')
            for move in board.legal_moves():
                tmp = board.copy()
                tmp.push(move) 
                # nodes+=1
                score = alphabeta(tmp, depth-1, False, alpha, beta, return_move=False)
                if score > maxscore:
                    bestmove = move
                    maxscore = score
                del tmp
                alpha = max(alpha, score)
                if beta<=alpha: break
            if not return_move: return maxscore
        else:
            minscore = 100000 #float('inf')
            for move in board.legal_moves():
                tmp = board.copy()
                tmp.push(move)
                # nodes+=1
                score = alphabeta(tmp, depth-1, True, alpha, beta, return_move=False)
                if score < minscore:
                    bestmove = move
                    minscore = score
                del tmp
                beta = min(beta, score)
                if beta<=alpha: break
            if not return_move: return minscore
        # print("nodes_per_sec: ", nodes/(time.time()-time_start))
        return bestmove

    # fen = '3r1b1r/pp2p1p1/4p1B1/R3P3/1Pk2R1p/7P/1P4P1/1N1Q2K1 b - - 43 44'
    # print(minmax(Board(fen=fen), 3, False))
    # quit()    
    # import chess.engine as engine
    # import chess    
    # stockfish = engine.SimpleEngine.popen_uci(r"C:\ThefCraft\global-env-exe\stockfish\stockfish-windows-x86-64-avx2.exe")
    
    class ChessGame:
        def __init__(self, master):
            self.master = master
            self.master.title("Chess Game")

            self.board = Board(fen=fen)
            
            self.selected_square = None
            self.size = 500
            self.board.set_svg_size(size=self.size)
            self.canvas = tk.Canvas(self.master, width=self.size, height=self.size)
            self.canvas.pack()

            self.draw_board()

            self.canvas.bind("<Button-1>", self.on_square_click)

        def draw_board(self):
            board_svg = self.board._repr_svg_()
            png_data = self.svg_to_png(board_svg)
            img = Image.open(BytesIO(png_data))
            self.board_image = ImageTk.PhotoImage(img)
            self.canvas.create_image(0, 0, anchor=tk.NW, image=self.board_image)

            if self.selected_square is not None:
                self.highlight_square(self.selected_square, color="yellow")

            if self.selected_square is not None:
                squares = self.board.legal_move_by_square(pos=self.selected_square)
                # print([get_square_name_from_pos(move) for move in legal_moves])
                for square in squares:
                    # TODO red for enpassant square
                    if self.board.piece_at(square):
                        self.highlight_square(square, color="red")
                    else:
                        self.highlight_square(square, color="green")
            self.master.update()
                                          
        def highlight_square(self, square, color):
            file = square & 7
            rank = 7 - (square >> 3)
            x0, y0 = file * self.size/8, rank * self.size/8
            x1, y1 = x0 + self.size/8, y0 + self.size/8
            x, y = (x0+x1)/2, (y0+y1)/2
            r = self.size / 32
            self.canvas.create_oval(x-r, y-r, x+r, y+r, fill=color, outline="", tags="highlight")
        def ai(self, player=False):
            # move = minmax(self.board, 3, player)
            move = alphabeta(self.board, 3, player)
            if move:
                self.board.push(move)
                # observer.push(chess.Move.from_uci(repr(move).lower()))
            else:
                print("FEN: ", self.board.fen())
                raise ValueError("FEN p", self.board.fen())
            self.draw_board()
        def svg_to_png(self, svg_data):
            png_data = cairosvg.svg2png(bytestring=svg_data)
            return png_data
        def on_square_click(self, event):
            # m1 = ' '.join(sorted([repr(move) for move in self.board.legal_moves()]))
            # m2 = ' '.join(sorted([move.uci() for move in observer.legal_moves]))
            print(self.board.fen())
            # print(observer.fen())
            # if m1 != m2: raise ValueError("MOVES DOES NOT match: ", self.board.fen())
            # print("legal_moves: ", m1)
            # print("legal_moves: ", m2)
            # print("GAME OVER: ", self.board.is_game_over(force=False), observer.is_game_over())
            print("GAME OVER: ", self.board.is_game_over(force=True))
            
            if self.board.is_game_over(force=True): return
            # if observer.is_game_over(): return
            
            if self.board.turn():
                self.ai(player=False)
                # result = stockfish.play(chess.Board(fen=self.board.fen()), chess.engine.Limit(time=0.0000001))  # Adjust the time limit as needed
                # print("STOCKfish: ", result.move.uci())
                
                # self.board.push(Move.from_uci(result.move.uci() if result.move.promotion else result.move.uci()))
                # observer.push(result.move)
            else:
                self.ai()
            
            # self.canvas.delete("highlight")
            self.draw_board()
            self.on_square_click(event)
            return
            square_size = self.size // 8
            file = event.x // square_size
            rank = 7 - (event.y // square_size)
            call_ai = False
            square = rank * 8 + file
            if self.selected_square is None:
                self.selected_square = square
            else:
                if square == self.selected_square:
                    self.selected_square = None
                else:
                    legal_moves = list(self.board.legal_moves())
                    for move in legal_moves:
                        if move.from_square == self.selected_square and move.to_square == square:
                            self.board.push_move_by_square(move.from_square, move.to_square)
                            # result = stockfish.play(chess.Board(fen=self.board.fen()), chess.engine.Limit(time=0.00001))  # Adjust the time limit as needed
                            # self.board.push(Move.from_uci(result.move.uci()))
                            call_ai = True
                            break
                    self.selected_square = None
            self.canvas.delete("highlight")
            self.draw_board()
            print("----------------------------------------------------------------")
            print(self.board.__repr__())
            if call_ai: self.ai()


    root = tk.Tk()
    game = ChessGame(root)
    root.mainloop()
