import sys
import time
from read import readInput
from write import writeOutput
import pickle
import random
import logging
import os
import json


from host import GO

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

class MyPlayer():
    def __init__(self):
        self.type = 'q'
        self.q_values = {}
        self.WIN_REWARD = 20
        self.LOSS_REWARD = 0.0
        self.DRAW_REWARD = 5
        self.alpha = 0.3
        self.gamma = 0.9
        self.epsilon = 0.9
        self.min_epsilon = 0.01
        self.decay_rate = 0.9969
        self.initial_value = 0
        self.history_states = []
        self.buffer_size = 13
        self.piece_type = None        
        self.max_depth = 5  # Set the maximum depth for alpha-beta pruning
        self.time_limit = 8  # Set a time limit for decision making
        self.previous_board = None
    def save_history_states(self):
        filename = f'history_states_{self.piece_type}.pkl'
        with open(filename, 'wb') as f:
            pickle.dump(self.history_states, f)

    def load_history_states(self):
        filename = f'history_states_{self.piece_type}.pkl'
        if os.path.exists(filename):
            with open(filename, 'rb') as f:
                self.history_states = pickle.load(f)
        else:
            self.history_states = []    

    

    def get_input(self, go, piece_type):
        #print(f"get_input called for piece_type {piece_type}")
        if self.is_new_game(piece_type, go.previous_board, go.board):
            
            self.reset_history(piece_type)  
            
        self.previous_board = go.previous_board
        self.go = go
        self.piece_type = piece_type
        #print(f"current piece_type: {self.piece_type}")
        self.epsilon = self.load_epsilon()
        self.load_history_states()
        #print(f"Current length of history_states at start of get_input: {len(self.history_states)}")
        self.load_q_values(f'q_values_{self.piece_type}.pkl')    
        self.epsilon = max(self.min_epsilon, self.epsilon * self.decay_rate)
        print(f"Current epsilon: {self.epsilon}")
        self.save_epsilon()
        
        # Always choose center for Black's first move
        if piece_type == 1 and all(all(cell == 0 for cell in row) for row in go.board):
            center = go.size // 2
            if go.valid_place_check(center, center, piece_type):
                state = self.board_to_state(go.board)
                self.history_states.append((state, (center, center)))
                return center, center
        
        state = self.board_to_state_original(go.board)
        valid_moves = self.get_valid_moves(go)
        if not valid_moves or valid_moves == ["PASS"]:
            self.history_states.append((state, "PASS"))
            return "PASS"
        
        best_move = None
        depth = 1  
        
        if random.random() < self.epsilon:
            best_move = random.choice(valid_moves)
            logging.info(f"Exploring: selected random move {best_move}")
        else:
        
            best_move = None
            best_score = float('-inf')
            start_time = time.time()

            while depth <= self.max_depth:                        
                #logging.debug(f"Searching at depth: {depth}")
                current_depth_start_time = time.time()
                ordered_moves = self.order_moves(go, valid_moves, piece_type,depth)
                #logging.debug(f"Number of ordered moves: {len(ordered_moves)}")

                for move in ordered_moves:
                    if move == "PASS":
                        continue
                    go_copy = go.copy_board()
                    go_copy.place_chess(move[0], move[1], piece_type)
                    go_copy.remove_died_pieces(3 - piece_type)
                    
                    score = self.alpha_beta_pruning(go_copy, depth, float('-inf'), float('inf'), False, piece_type, start_time)

                    if score is None:  # Time limit exceeded
                        #logging.debug("Time limit exceeded during alpha-beta pruning")
                        break

                    if score > best_score:
                        best_score = score
                        best_move = move

                    if time.time() - start_time > self.time_limit:
                        return best_move if best_move != "PASS" else "PASS"
                    

                    # Increase depth for the next iteration
                depth_time = time.time() - current_depth_start_time
                #logging.debug(f"Depth {depth} completed. Time taken: {depth_time:.2f}s") 
                depth += 1 
        
        self.history_states.append((state, best_move))

        if len(self.history_states) > self.buffer_size:
            self.history_states.pop(0)

        self.save_history_states()
        return best_move

    def alpha_beta_pruning(self, go, depth, alpha, beta, is_maximizing, piece_type, start_time):
        if time.time() - start_time > self.time_limit:
            return None
            
        if depth == 0 or go.game_end(piece_type):
            if go.game_end(piece_type):
                winner = go.judge_winner()
                if winner == piece_type:
                    return float('inf')  
                elif winner == 3 - piece_type:
                    return float('-inf')  
                else:
                    return 0             
            state = self.board_to_state(go.board)
            return self.evaluate_state(go, state, None, piece_type)
        
        current_player = piece_type if is_maximizing else 3 - piece_type
        best_score = float('-inf') if is_maximizing else float('inf')
        valid_moves = self.get_valid_moves(go) 
        if not valid_moves or valid_moves == ["PASS"]:
            return self.evaluate_state(go, self.board_to_state(go.board), "PASS", current_player)               

        ordered_moves = self.order_moves(go, valid_moves, current_player,depth)

                        
        
        for move in ordered_moves:
            i, j = move
            go_copy = go.copy_board()
            go_copy.place_chess(i, j, current_player)
            go_copy.remove_died_pieces(3 - current_player)
            score = self.alpha_beta_pruning(go_copy, depth - 1, alpha, beta, not is_maximizing, piece_type, start_time)

            if score is None:
                return None

            if is_maximizing:
                best_score = max(best_score, score)
                alpha = max(alpha, best_score)
            else:
                best_score = min(best_score, score)
                beta = min(beta, best_score)

            if beta <= alpha:
                break
        
        return best_score
    
    def order_moves(self, go, moves, piece_type,depth):
        move_scores = []
        state = self.board_to_state(go.board)
        for move in moves:
            if move == "PASS":
                score = self.get_q_value(state, move)
            else:
                heuristic_score = self.evaluate_move(go, move, piece_type)
                q_value = self.get_q_value(state, move)     
                #logging.debug(f"Heuristic score: {heuristic_score}, Q-value: {q_value}")           
                score = heuristic_score/2 + q_value
            move_scores.append((score, move))

        move_scores.sort(reverse=True)
        if len(moves) <= 5:  # For very few moves, consider all
            return [move for score, move in move_scores]
            
        if depth <= 2:  # Early depths, consider more moves
            limit = len(move_scores)
        elif depth <= 3:
            limit = max(10, len(move_scores) // 2)
        elif depth <= 4:
            limit = max(8, len(move_scores) // 3)
        else:  # Deep searches, consider fewer moves
            limit = max(5, len(move_scores) // 4)
        
        ordered_moves = [move for score, move in move_scores[:limit]]
       
        return ordered_moves

    def evaluate_move(self, go, move, piece_type):
        if move == "PASS":
            return 0  

        i, j = move
        go_copy = go.copy_board()
        go_copy.place_chess(i, j, piece_type)
        captured_stones = len(go_copy.remove_died_pieces(3 - piece_type))
        neighbors = go.detect_neighbor(i, j)
        empty_neighbors = sum(1 for ni, nj in neighbors if go.board[ni][nj] == 0)
        enemy_neighbors = sum(1 for ni, nj in neighbors if go.board[ni][nj] == 3 - piece_type)
        status = 0
        corners = [(0, 0), (0, 4), (4, 0), (4, 4)]
        if (i, j) in corners:
             
            status = 1
        elif i == 0 or i == 4 or j == 0 or j == 4:
            
            status = 2

        connection_bonus = self.assess_connection_strength(go_copy, piece_type) - self.assess_connection_strength(go, piece_type)
        
        unconditional_life = self.count_unconditional_life(go_copy, piece_type) - self.count_unconditional_life(go, piece_type)
        influence_change = self.calculate_influence(go_copy, piece_type) - self.calculate_influence(go, piece_type)
        liberty_change = self.count_liberties(go_copy, piece_type) - self.count_liberties(go, piece_type)
        
        
        non_empty_spaces = 0
        for i in range(5):  
            for j in range(5):
                if go_copy.board[i][j] != 0:
                    non_empty_spaces += 1
        total_spaces = 25
       
        game_progress = non_empty_spaces / total_spaces
        
        if game_progress < 0.3:  # Early game
           
            liberty_weight = 3
            influence_weight = 2.0
            connection_weight = 2.0  # Increased weight for connections           
            
            if(piece_type == 1):
                capture_weight = 2.5
                liberty_weight = 4.0
            else:
                capture_weight = 0.5
        elif game_progress < 0.6:  # Mid game
          
            liberty_weight = 2.5
            influence_weight = 1.5
            connection_weight = 3.0  # Increased weight
          

            if(piece_type == 1):
                capture_weight = 4.5
                liberty_weight = 3.5
            else:
                capture_weight = 1
        else:  # Late game
            
            liberty_weight = 2
            influence_weight = 1.0
            connection_weight = 3.5  # Further increased
 
            if(piece_type == 1):
                capture_weight = 3.5
                liberty_weight = 3.0
            else:
                capture_weight = 1
        

      
        total_score = (
            connection_weight * connection_bonus +
            influence_weight * influence_change +
            liberty_weight * liberty_change +
            capture_weight * captured_stones +            
            + unconditional_life*2.5
        )
        
        
        score_change = go_copy.score(piece_type) - go.score(piece_type)   
        if(piece_type == 1):
            if(go.score(piece_type) < go.score(3 - piece_type)+2.5):
                total_score += captured_stones*3
            
            if(score_change < 0):
                total_score += score_change*5
        else:
            if(score_change < 0):
                total_score += score_change*5
            if(go.score(piece_type) < go.score(3 - piece_type)-2.5):
                total_score += captured_stones*3
            
        if status == 1 and enemy_neighbors == 1 and empty_neighbors == 1:
            total_score -= 10  
        elif status == 2 and enemy_neighbors == 2 and empty_neighbors == 1:
            total_score -= 10  
        elif status == 0 and enemy_neighbors == 3 and empty_neighbors == 1:
            total_score -= 15  
        return total_score
    
    
    def calculate_influence(self, go, piece_type):
        influence = 0
        for i in range(go.size):
            for j in range(go.size):
                if go.board[i][j] == piece_type:
                    influence += self.calculate_stone_influence(go, i, j, piece_type)
        return influence

    def calculate_stone_influence(self, go, i, j, piece_type):
        influence = 0
        for di in range(-2, 3):
            for dj in range(-2, 3):
                ni, nj = i + di, j + dj
                if 0 <= ni < go.size and 0 <= nj < go.size:
                    distance = max(abs(di), abs(dj))
                    if distance == 0:
                        continue
                    if go.board[ni][nj] == 0:
                        influence += 1 / distance
                    elif go.board[ni][nj] == 3 - piece_type:
                        influence -= 1 / distance
        return influence
    
    def assess_eye_potential(self, go, piece_type):
        eye_potential = 0
        for i in range(go.size):
            for j in range(go.size):
                if go.board[i][j] == 0:
                    eye_potential += self.assess_eye_at_point(go, i, j, piece_type)
        return eye_potential

    def assess_eye_at_point(self, go, i, j, piece_type):
        surrounding_stones = 0
        for ni, nj in go.detect_neighbor(i, j):
            if go.board[ni][nj] == piece_type:
                surrounding_stones += 1
        if surrounding_stones == 4:
            return 1
        elif surrounding_stones == 3:
            return 0.5
        return 0
    
    def assess_connection_strength(self, go, piece_type):
        connection_strength = 0
        for i in range(go.size):
            for j in range(go.size):
                if go.board[i][j] == piece_type:
                    connection_strength += self.assess_stone_connection(go, i, j, piece_type)
        return connection_strength

    def assess_stone_connection(self, go, i, j, piece_type):
        connection_value = 0
        for ni, nj in go.detect_neighbor(i, j):
            if go.board[ni][nj] == piece_type:
                connection_value += 1
            elif go.board[ni][nj] == 0:
                connection_value += 0.5
        return connection_value

    def assess_cutting_potential(self, go, piece_type):
        cutting_potential = 0
        for i in range(go.size):
            for j in range(go.size):
                if go.board[i][j] == 0:
                    cutting_potential += self.assess_cut_at_point(go, i, j, piece_type)
        return cutting_potential

    def assess_cut_at_point(self, go, i, j, piece_type):
        opponent = 3 - piece_type
        opponent_neighbors = 0
        empty_neighbors = 0
        for ni, nj in go.detect_neighbor(i, j):
            if go.board[ni][nj] == opponent:
                opponent_neighbors += 1
            elif go.board[ni][nj] == 0:
                empty_neighbors += 1
        if opponent_neighbors >= 2 and empty_neighbors >= 1:
            return 1
        return 0

    def load_epsilon(self):
        try:
            with open(f'epsilon_{self.piece_type}.pkl', 'rb') as f:
                return pickle.load(f)
        except FileNotFoundError:
            self.save_epsilon()
            return 1.0  

    def save_epsilon(self):
        with open(f'epsilon_{self.piece_type}.pkl', 'wb') as f:
            pickle.dump(self.epsilon, f)

    

    def board_to_state_original(self, board):
        black_bitboard = 0
        white_bitboard = 0
        size = len(board)
        
        for i in range(size):
            for j in range(size):
                if board[i][j] == 1:  # Black stone
                    black_bitboard |= (1 << (i * size + j))
                elif board[i][j] == 2:  # White stone
                    white_bitboard |= (1 << (i * size + j))
        
        return (black_bitboard, white_bitboard)   

    def board_to_state(self, board):
        symmetries = self.get_symmetries(board)
        bitboard_states = []
        
        for sym in symmetries:
            black_bitboard = 0
            white_bitboard = 0
            size = len(sym)
            
            for i in range(size):
                for j in range(size):
                    if sym[i][j] == 1:
                        black_bitboard |= (1 << (i * size + j))
                    elif sym[i][j] == 2:
                        white_bitboard |= (1 << (i * size + j))
            
            bitboard_states.append((black_bitboard, white_bitboard))
        
        return min(bitboard_states)
    
    def get_symmetries(self, board):
        symmetries = [board]
        
        for i in range(1, 4):
            rotated = list(zip(*board[::-1]))
            symmetries.append(rotated)
            board = rotated
        
        flipped = [row[::-1] for row in board]
        symmetries.append(flipped)
        for i in range(1, 4):
            rotated = list(zip(*flipped[::-1]))
            symmetries.append(rotated)
            flipped = rotated

        return [tuple(tuple(row) for row in sym) for sym in symmetries]
    
    def count_liberties(self, go, piece_type):
        liberties = 0
        visited = set()
        for i in range(go.size):
            for j in range(go.size):
                if go.board[i][j] == piece_type and (i, j) not in visited:
                    group_liberties = self.get_group_liberties(go, i, j, piece_type, visited)
                    liberties += len(group_liberties)
        return liberties

    def get_group_liberties(self, go, i, j, piece_type, visited):
        group = set()
        liberties = set()
        stack = [(i, j)]
        while stack:
            x, y = stack.pop()
            if (x, y) in visited:
                continue
            visited.add((x, y))
            group.add((x, y))
            for nx, ny in go.detect_neighbor(x, y):
                if go.board[nx][ny] == 0:
                    liberties.add((nx, ny))
                elif go.board[nx][ny] == piece_type:
                    stack.append((nx, ny))
        return liberties
   
    def get_q_value(self, state, action):
        if state not in self.q_values:
            self.q_values[state] = {}
        if action not in self.q_values[state]:
            self.q_values[state][action] = self.initial_value
        return self.q_values[state][action]

    def get_valid_moves(self, go):
        valid_moves = []
        for i in range(go.size):
            for j in range(go.size):
                if go.valid_place_check(i, j, self.piece_type, test_check=True):
                    valid_moves.append((i, j))
        if not valid_moves:
            valid_moves.append("PASS")
        return valid_moves
    
    def count_potential_captures(self, go, piece_type):
        potential_captures = 0
        for i in range(go.size):
            for j in range(go.size):
                if go.board[i][j] == 3 - piece_type:  # opponent's stone
                    if len(go.detect_neighbor(i, j)) == 1:  # only one liberty
                        potential_captures += 1
        return potential_captures
    
    def count_corner_side_control(self, go, piece_type):
        control = 0
        board_size = go.size
        corners = [(0, 0), (0, board_size - 1), (board_size - 1, 0), (board_size - 1, board_size - 1)]
        for i, j in corners:
            if go.board[i][j] == piece_type:
                control += 3  # Adjust as needed
        for i in range(board_size):
            for j in range(board_size):
                if (i, j) not in corners and (i == 0 or i == board_size - 1 or j == 0 or j == board_size - 1):
                    if go.board[i][j] == piece_type:
                        control += 2  # Adjust as needed
        return control

    def learn(self, final_reward,  piece_type):
        go = GO(5)  # Assuming the board size is 5x5
        go_dummy = GO(5)
        self.go = go
        self.piece_type = piece_type
        
    
        
        total_moves = len(self.history_states)
        
        if total_moves == 0:
            print("Warning: No moves in history to learn from.")
            return
        
        last_state, last_action = self.history_states[-1]
          # Create a new GO object for the previous state
        go.board = self.state_to_board(last_state)
        go_dummy.board = self.state_to_board(last_state)
        symmetry_last_state = self.board_to_state(go_dummy.board)
        #print("last board:", go.board)
        current_q = self.get_q_value(symmetry_last_state, last_action)
        new_q = final_reward
        self.q_values[symmetry_last_state][last_action] = new_q
        
        
        for index, (state, action) in enumerate(reversed(self.history_states[:-1])):
            total_spaces = 12            
            game_progress = (12-index) / total_spaces
            go_before = GO(5)  # Create a new GO object for the previous state
            go_before.board = self.state_to_board(state)         
            symmetry_state = self.board_to_state(go_before.board)
            #print("Before Board:", go_before.board)
            
            
            territory_factor = (self.count_unconditional_territory(go, piece_type) - 
                           self.count_unconditional_territory(go_before, piece_type))                   
            
            net_liberty_factor = (self.count_liberties(go, piece_type) - self.count_liberties(go_before, piece_type)) - \
                         (self.count_liberties(go_before, 3 - piece_type) - self.count_liberties(go, 3 - piece_type))
            
            unconditional_life_factor = self.count_unconditional_life(go, piece_type) - self.count_unconditional_life(go_before, piece_type)

            
            opponent_score_before, opponent_score_after = go_before.score(3 - piece_type), go.score(3 - piece_type)
            my_score_before, my_score_after = go_before.score(piece_type), go.score(piece_type)    
            my_score_change, opponent_score_change = my_score_after - my_score_before, opponent_score_before - opponent_score_after
            net_score_change = my_score_change - opponent_score_change
            after_score_difference = my_score_after - opponent_score_after
            catch_bonus = 0
            if piece_type == 1:  # Black         
                if after_score_difference > 3.5:
                    catch_bonus += unconditional_life_factor + opponent_score_change
                else:
                    catch_bonus += opponent_score_change * 5
            else:                  
                if after_score_difference > -1.5:
                    catch_bonus += unconditional_life_factor + my_score_change
                else: 
                    catch_bonus += opponent_score_change * 4
 
            if game_progress < 0.3:  # Early game
                territory_weight = 1.0
                score_weight = 1.0
                liberty_weight = 2.5
                capture_weight = 1.0
                unconditional_life_weight = 1.5
                if(piece_type == 1):
                    score_weight += 2.0
                    liberty_weight += 2
                    capture_weight = 3.0
                    unconditional_life_weight = 2.0

                
            
            
            

            elif game_progress < 0.6:  # Mid game
                territory_weight = 1.5
                score_weight = 2.0
                liberty_weight = 2.5
                capture_weight = 1.5
                unconditional_life_weight = 2.3
                if(piece_type == 1):
                    score_weight += 2.0
                    liberty_weight += 1.5
                    capture_weight = 5.0
                    unconditional_life_weight = 3.0
        
                
                
            
            else:  # Late game
                territory_weight = 2.0
                score_weight = 3.0
                liberty_weight = 1.5
                capture_weight = 1.0
                unconditional_life_weight = 3.0
                if(piece_type == 1):
                    score_weight += 2.0
                    liberty_weight += 2.0
                    capture_weight = 4.0
                    unconditional_life_weight = 3.5
   
                
  
            immediate_reward = (territory_factor * territory_weight
                                + (net_liberty_factor * liberty_weight) 
                                + unconditional_life_factor * unconditional_life_weight
                                + (net_score_change * score_weight)
                                + catch_bonus * capture_weight                                 
                                )
            
            # print("Index:", index)
            # print("Territory Factor:", territory_factor)
            # print("Liberty Factor:", net_liberty_factor)      
            # print("Captured Stones:", captured_factor)          
            # print("Net Score Change:", net_score_change * 2)
            # print("Immediate Reward:", immediate_reward)
            # print("------------------------")            
            # if(piece_type == 1):          
            #     if score_factor > 2:
            #         immediate_reward += 0.5*(territory_factor + liberty_factor + unconditional_life_factor)
            #     else:
            #         immediate_reward += opponent_score_change
            # else:
            #     if(my_score_change < 0):
            #         immediate_reward += my_score_change*2

            #     if score_factor > -1:
            #         immediate_reward += 1
                                    

            immediate_reward = min(max(immediate_reward, -10.0), 10.0)

            old_q = self.get_q_value(symmetry_state, action)
            
            future_q = max(self.get_q_value(symmetry_state, a) for a in self.get_valid_moves(go))
            new_q = (1-self.alpha)*old_q + self.alpha * (immediate_reward + self.gamma*future_q)            
            self.q_values[symmetry_state][action] = new_q
            # print(f"Move {total_moves - index - 1}:")
            # print(f"  Immediate reward: {immediate_reward:.4f}")
            # print(f"  Old Q-value: {old_q:.4f}")
            # print(f"  New Q-value: {new_q:.4f}")
            # print("------------------------")
            go = go_before
            

        self.history_states = []
        print(f"Learned from {total_moves} moves.")

    def state_to_board(self, state, size=5):
        black_bitboard, white_bitboard = state
        board = [[0 for _ in range(size)] for _ in range(size)]
        for i in range(size):
            for j in range(size):
                if black_bitboard & (1 << (i * size + j)):
                    board[i][j] = 1
                elif white_bitboard & (1 << (i * size + j)):
                    board[i][j] = 2
        return board

    def evaluate_captures(self, go, go_before, piece_type):
        opponent = 3 - piece_type
        liberties_before = self.count_liberties(go_before, opponent)
        liberties_after = self.count_liberties(go, opponent)
        
        # A decrease in opponent's liberties is good for the current player
        liberty_difference = liberties_before - liberties_after
        
        
        # Combine both metrics
        return liberty_difference

    def count_unconditional_territory(self, go, piece_type):
        territory = 0
        for i in range(go.size):
            for j in range(go.size):
                if go.board[i][j] == 0 and self.is_unconditional_territory(go, i, j, piece_type):
                    territory += 1
       
            
        return territory    

          

    def is_unconditional_territory(self, go, i, j, piece_type):      
        for ni, nj in go.detect_neighbor(i, j):
            if go.board[ni][nj] == 0 and not self.is_surrounded_by(go, ni, nj, piece_type):
                return False
        return self.is_surrounded_by(go, i, j, piece_type)

    def is_surrounded_by(self, go, i, j, piece_type):
        for ni, nj in go.detect_neighbor(i, j):
            #if go.board[ni][nj] != piece_type and go.board[ni][nj] != 0:
            if go.board[ni][nj] != piece_type:
                return False
        return True

    def count_unconditional_life(self, go, piece_type):
        life_count = 0
        visited = set()
        for i in range(go.size):
            for j in range(go.size):
                if go.board[i][j] == piece_type and (i, j) not in visited:
                    group = self.find_group(go, i, j, piece_type, visited)
                    if self.has_unconditional_life(go, group, piece_type):
                        life_count += len(group)        
        return life_count

    def find_group(self, go, i, j, piece_type, visited):
        group = set([(i, j)])
        stack = [(i, j)]
        visited.add((i, j))
        while stack:
            x, y = stack.pop()
            for nx, ny in go.detect_neighbor(x, y):
                if go.board[nx][ny] == piece_type and (nx, ny) not in visited:
                    group.add((nx, ny))
                    stack.append((nx, ny))
                    visited.add((nx, ny))
        return group

    def has_unconditional_life(self, go, group, piece_type):
        
        eyes = self.find_eyes(go, group, piece_type)
        return len(eyes) >= 2

    def find_eyes(self, go, group, piece_type):
        eyes = set()
        for i, j in group:
            for ni, nj in go.detect_neighbor(i, j):
                if go.board[ni][nj] == 0 and self.is_eye(go, ni, nj, piece_type):
                    eyes.add((ni, nj))
        return eyes

    def is_eye(self, go, i, j, piece_type):
    # Check if the point (i, j) is surrounded by the same piece type
        if not self.is_surrounded_by(go, i, j, piece_type):
            return False

        # Determine if (i, j) is in the corner, on the side, or in the center
        size = go.size
        is_corner = (i == 0 or i == size - 1) and (j == 0 or j == size - 1)
        is_side = (i == 0 or i == size - 1 or j == 0 or j == size - 1) and not is_corner

        # Check diagonals for corner, side, and center
        diagonals = [(i-1, j-1), (i-1, j+1), (i+1, j-1), (i+1, j+1)]
        diagonal_count = 0
        for di, dj in diagonals:
            if 0 <= di < size and 0 <= dj < size:
                if go.board[di][dj] == piece_type:
                    diagonal_count += 1

        # Apply the rules based on position
        if is_corner:
            # Corner: At least 1 diagonal must match
            return diagonal_count >= 1
        elif is_side:
            # Side: At least 2 diagonals must match
            return diagonal_count >= 2
        else:
            # Center (2/4 rule): At least 3 diagonals must match
            return diagonal_count >= 3

    def save_q_values(self, filename):
        with open(filename, 'wb') as f:
            pickle.dump(self.q_values, f)

    def load_q_values(self, filename):
        try:
            with open(filename, 'rb') as f:
                self.q_values = pickle.load(f)
        except FileNotFoundError:
            self.q_values = {}

    def learn_and_save(self, final_reward, filename):
        self.learn(final_reward)
        self.save_q_values(filename)

    def evaluate_state(self, go, state, action, piece_type):
        q_value = self.get_q_value(state, action) if action is not None else 0
        
        # go_copy = go.copy_board()
        # if action is not None and action != "PASS":
        #     go_copy.place_chess(action[0], action[1], piece_type)
        #     go_copy.remove_died_pieces(3 - piece_type)
        
        # territory_factor = self.count_unconditional_territory(go_copy, piece_type) * 0.05
        # liberty_factor = self.count_liberties(go_copy, piece_type) * 0.01
        
        
        # if piece_type == 1:  # Black
        #     # Fixed komi factor
        #     opponent_liberty_factor = (self.count_liberties(go, 3-piece_type) - 
        #                                self.count_liberties(go_copy, 3-piece_type)) * 0.02
        #     potential_capture_factor = self.count_potential_captures(go_copy, piece_type) * 0.02
            
            
        #     evaluation = (q_value 
        #                   + territory_factor 
        #                   + liberty_factor 
                         
        #                   + opponent_liberty_factor
        #                   + potential_capture_factor)
        # else:  # White
        #     life_factor = self.count_unconditional_life(go_copy, piece_type) * 0.01
        #     evaluation = q_value + territory_factor + liberty_factor + life_factor 
        
        return q_value

    def is_new_game(self, piece_type, previous_board, board):
        if self.previous_board is None:
            return True
        
        # Check if previous_board is empty
        prev_board_empty = all(previous_board[i][j] == 0 for i in range(len(previous_board)) for j in range(len(previous_board)))
        
        # Check if the current board has exactly one stone (for white's first move)
        one_stone_on_board = sum(board[i][j] != 0 for i in range(len(board)) for j in range(len(board))) == 1
        
        # It's a new game if:
        # 1. Both stored previous_board and current previous_board are empty (black's first move)
        # 2. Previous board is empty and current board has exactly one stone (white's first move)
        return (all(self.previous_board[i][j] == 0 for i in range(len(previous_board)) for j in range(len(previous_board))) and 
                prev_board_empty and 
                (piece_type == 1 or (piece_type == 2 and one_stone_on_board)))

    def reset_history(self, piece_type):
        filename = f'history_states_{piece_type}.pkl'
        if os.path.exists(filename):
            os.remove(filename)
        if piece_type == self.piece_type:
            self.history_states = []
        print(f"History states reset for piece type {piece_type}")

if __name__ == "__main__":    
    N = 5
    piece_type, previous_board, board = readInput(N)
    go = GO(N)
    go.set_board(piece_type, previous_board, board)
    player = MyPlayer()
    player.previous_board = previous_board
    action = player.get_input(go, piece_type)
    writeOutput(action)

    




















