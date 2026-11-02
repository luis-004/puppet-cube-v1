#include "Solver.hpp"
namespace { // needed when using a closed list
    struct StateHash {
        size_t operator()(const Puppet::State& s) const {
            const unsigned char* p = reinterpret_cast<const unsigned char*>(&s);
            uint64_t h = 1469598103934665603ull; // FNV-1a
            for(size_t i = 0; i < sizeof(s); i++){
                h ^= p[i];
                h *= 1099511628211ull;
            }
            return static_cast<size_t>(h);
        }
    };
    struct StateEq {
        bool operator()(const Puppet::State& a, const Puppet::State& b) const {
            return std::memcmp(&a, &b, sizeof(a)) == 0;
        }
    };
}

Solver::Solver(const std::string& shape_rank_file, const std::string& valid_moves_file, const std::string& CEO_table_file, const std::string& SEP_table_file){
    table.load_table(table.shape_ranks, shape_rank_file);
    table.load_table(table.shape_moves, valid_moves_file);
    table.load_large_table(table.CEO_table, CEO_table_file);
    table.load_large_table(table.SEP_table, SEP_table_file);
}

// picks the maximum out of the two values given by the PDBs
uint8_t Solver::getHeuVal(const Puppet::State& state){
    auto EdgeIdxs = puppet.getEdgeRanks(state);
    auto ShapeIdx = table.shape_ranks[puppet.getShapeKey(state)];
    uint8_t cfVal = table.CEO_table[(ShapeIdx * NUM_CORNER_CYCLES + puppet.getCornerRank(state)) * NUM_EDGE_ORIENTATIONS + EdgeIdxs.second]; // puppet.getFlipIdx(state)
    uint8_t seVal = table.SEP_table[ShapeIdx * NUM_EDGE_PERMUTATIONS + EdgeIdxs.first]; // puppet.getEPIdx(state)
    //uint8_t cfVal = table.table_CF[table.CEO_index(state)];
    //uint8_t seVal = table.table_SE[table.SEP_index(state)];
    return cfVal > seVal ? cfVal : seVal;
}

std::vector<Move> Solver::astar(const Puppet::State& start){
    struct Node{ // 68 bytes
        Puppet::State state;
        std::array<Move, 50> move_sequence;
        uint8_t g;
        uint8_t h;
    };
    Node next;
    Puppet::State state = start;
    uint8_t bucket = 0; // bucket iterator
    std::array<std::vector<Node>, 3> open;
    //std::unordered_map<Puppet::State, uint8_t, StateHash, StateEq> closed;
    open[0].push_back({state, {}, 0, getHeuVal(start)});
    size_t nodes_gen = 1;
    size_t max_nodes = 1;
    auto start_time = std::chrono::high_resolution_clock::now();

    while(true){
        size_t size = open[0].size() + open[1].size() + open[2].size();
        if(size > max_nodes){
            max_nodes = size;
        }
        if(open[bucket].empty()){ // current best bucket is empty -> switch to next best bucket
            bucket = (bucket + 1) % 3; 
            continue;
        }
        Node cur = open[bucket].back(); // pop a node from the best bucket
        open[bucket].pop_back();
        state = cur.state;
        uint8_t last_face = cur.g > 0 ? getFace(cur.move_sequence[cur.g-1]) : 255;

        uint16_t valid_turns = table.shape_moves[puppet.getShapeKey(state)]; // valid moves for the state of the node
        
        // iterate over all valid moves 
        uint8_t move_idx;
        while((move_idx = __builtin_ctz(valid_turns)) < NUM_MOVES){ // __builtin_ctz: get idx of least significant 1
            valid_turns &= valid_turns - 1; // remove picked move
            Move m = static_cast<Move>(move_idx);
            if(getFace(m) == last_face) continue; // avoid consecutive moves on same face
            next.state = state;
            puppet.turn(next.state, m);
            uint8_t h_next = getHeuVal(next.state);
            if(h_next == 0){ // goal state found
                std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start_time;
                std::vector<Move> alg;
                for(uint8_t i = 0; i < cur.g; i++){
                    alg.push_back(cur.move_sequence[i]);
                }
                alg.push_back(m);
                std::cout << " time:" << duration.count() <<" nodes_generated:" << nodes_gen << " node_size:" << sizeof(Node)
                << " maximum_nodes_stored:"<< max_nodes << " nodes/second:" << double(duration.count() > 0 ? nodes_gen / double(duration.count()) : 0.0) << " solution_length:" << int(cur.g + 1) << std::endl;
                //std::cout << am.translate(am.reverse(alg)) << "  " << int(cur.len + 1) << std::endl;
                //std::cout << duration.count() << "," << nodes_gen << "," << max_nodes << "," << int(cur.len + 1) << "," << std::endl;
                return alg;
            }
            next.move_sequence = cur.move_sequence;
            next.move_sequence[cur.g] = m;
            next.g = cur.g + 1;
            next.h = h_next;
            open[(bucket + 1 + h_next - cur.h)%3].push_back(next); // put the node in the right bucket
            nodes_gen++;
            /*uint8_t new_g = cur.g + 1;
            auto it = closed.find(next.state);
            if(it != closed.end()){
                continue;
            } else {
                closed.emplace(next.state, new_g);
            }
            next.move_sequence = cur.move_sequence;
            next.move_sequence[cur.g] = m;
            next.g = new_g;
            next.h = h_next;
            open[(bucket + 1 + h_next - cur.h)%3].push_back(next);
            nodes_gen++;*/
        }
    }
}

std::vector<Move> Solver::iterative_deepening(const Puppet::State& start, const size_t max_depth){
    auto start_time = std::chrono::high_resolution_clock::now();
    const int h0 = getHeuVal(start);
    if(h0 == 0) return {};
    struct Node{ // 19 bytes, 20 with padding
        Puppet::State state;
        uint16_t turns_to_try;
        uint8_t last_face;
    };
    std::vector<Node> open(max_depth + 1);
    std::vector<Move> path(max_depth + 1);
    uint8_t bound = h0; // set bound to initial estimate first
    size_t nodes_gen = 0;
    size_t states_visited = 1;
    while(bound <= int(max_depth)){
        int next_bound = INT_MAX;
        int depth = 0;
        open[0] = {start, table.shape_moves[puppet.getShapeKey(start)], 255};
        while(depth >= 0){
            Node& n = open[depth];
            if(n.turns_to_try == 0){
                depth--;
                continue;
            }
            uint8_t i = __builtin_ctz(n.turns_to_try); // pick next valid turn
            n.turns_to_try &= n.turns_to_try - 1; // remove picked turn
            Move m = static_cast<Move>(i);
            uint8_t face = getFace(m);
            if(face == n.last_face) continue; // avoid consecutive moves on same face
            Puppet::State next = n.state;
            puppet.turn(next, m);
            int next_h = getHeuVal(next);
            states_visited++;
            path[depth] = m; // extend the path
            if(next_h == 0){ // goal state found
                std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start_time;
                std::cout << " time:" << duration.count() <<" nodes_generated:" << nodes_gen
                << " maximum_nodes_stored:"<< depth << " nodes/second:" << double(duration.count() > 0 ? nodes_gen / double(duration.count()) : 0.0) 
                << " state_visited:" << states_visited
                << " solution_length:" << depth + 1 << std::endl;
                return std::vector<Move>(path.begin(), path.begin() + depth + 1);
            }
            int f = depth + 1 + next_h;
            if(f > bound){ // prune states with too high f
                next_bound = std::min(next_bound, f); // check if next bound should be smaller
                continue;
            }
            if(depth + 1 >= int(max_depth)) continue;
            open[depth + 1] = {next, table.shape_moves[puppet.getShapeKey(next)], face};
            nodes_gen++;
            depth++;
        }
        if(next_bound == INT_MAX) break;
        bound = next_bound;
    }
    return {};
}

// batch weighted A* implementation adopted by DeepCubeA without the socket communication, adapted for the PDB heuristics
std::vector<Move> Solver::bwas(const Puppet::State& start, const size_t batch_size, const double depthPenalty){
    struct Node {
        Puppet::State state;
        int depth;
        int parentMove;
        float cost;
        uint8_t heuristic;
        Node *parent;
        bool operator==(const Node &other) const {
            return state == other.state;
        }
    };
    struct Hash {
        size_t operator() (const Node *node) const {
            std::size_t seed = 0;
            boost::hash_range(seed, std::begin((node->state).corners), std::end((node->state).corners));
            boost::hash_range(seed, std::begin((node->state).edges), std::end((node->state).edges));
            return seed;
        }
    };
    struct NodePointerEq {
        bool operator () ( Node const *lhs, Node const *rhs ) const {
            return lhs->state == rhs->state;
        }
    };
    auto cmp = [depthPenalty](const Node* a, const Node* b){
        return a->cost > b->cost;
    };
    std::priority_queue<Node*, std::vector<Node*>, decltype(cmp)> open(cmp);
    std::unordered_set<Node*, Hash, NodePointerEq> closed;

    auto search_start_time = std::chrono::high_resolution_clock::now();
    double tExpand = 0, tClosed = 0, tPush = 0;
    Puppet::State state = start;
    //std::cout << "heuval: " << int(getHeuVal(state)) << std::endl;
	open.push(new Node{state,0,-1,0,0,NULL}); //Push root node to open
	closed.insert(new Node{state,0,-1,0,0,NULL}); //Add root node to seen
    int searchItr = 1;
	long numNodesGenerated = 1;
	bool isSolved = false;
    Node *solvedNode = NULL;
    while (isSolved == false) {
		//std::chrono::high_resolution_clock::time_point startTime, t1;

		//startTime = std::chrono::high_resolution_clock::now();

		// Remove from open
		int openSize = (int) open.size();
		int numPop = std::min(openSize, int(batch_size));
        if(numPop == 0){
            auto now = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = now - search_start_time;
            std::cout << " time:" << duration.count() << " tExpand:" << tExpand << " tClosed:" << tClosed << " tPush:" << tPush 
            << " iterations: " << searchItr <<" nodes_generated: " << numNodesGenerated
            << " nodes_per_second: " << double(duration.count() > 0 ?  numNodesGenerated / double(duration.count()) : 0.0)
            << " NO SOLUTION FOUND!!" << std::endl; 
            return {};
        }
		std::vector<Node*> popped;

		//t1 = std::chrono::high_resolution_clock::now();
		bool goal_node_found_prev = solvedNode != NULL;
		for (int i=0; i<numPop; i++) {
			Node *node = open.top();
			popped.push_back(node);
			open.pop();

			bool isSolved_itr = puppet.testSolved(node->state);
			if (isSolved_itr) {
			    if (batch_size == 1) {
				    solvedNode = node;
			        isSolved = true;
			    } else {
                    if (solvedNode == NULL) {
                        solvedNode = node;
                    } else if (solvedNode->cost > node->cost) {
                        solvedNode = node;
                    }
			    }
			    break;
			}
		}
		if (goal_node_found_prev && (popped[0]->cost >= solvedNode->cost)) {
		    isSolved = true;
		}
		
		auto t0 = std::chrono::high_resolution_clock::now();
        // Expand

		std::vector<Node*> children(popped.size()*NUM_MOVES);
        #pragma omp parallel for
		for (unsigned int i=0; i<popped.size(); i++) {
            uint16_t valid_moves = table.shape_moves[puppet.getShapeKey(popped[i]->state)];
            uint8_t move_idx;
            while((move_idx = __builtin_ctz(valid_moves)) < NUM_MOVES){
                valid_moves &= valid_moves - 1;
                Move m = static_cast<Move>(move_idx);
                if(getFace(m) == getFace(static_cast<Move>(popped[i]->parentMove))) continue; // avoid consecutive moves on same face
                Puppet::State next = popped[i]->state;
                puppet.turn(next, m);
                uint8_t heuval = getHeuVal(next);
                float cost = heuval + depthPenalty*((float) popped[i]->depth + 1); // calculate weighted evaluation function
                Node *node = new Node{next,popped[i]->depth + 1,move_idx,cost,heuval,popped[i]};
                children[i*NUM_MOVES + move_idx] = node;
            }
		}

        children.erase(std::remove(children.begin(), children.end(), nullptr), children.end()); // remove empty slots in children
		if(children.size() == 0){continue;}
        numNodesGenerated += children.size();

        auto t1= std::chrono::high_resolution_clock::now();
        tExpand += std::chrono::duration<double>(t1 - t0).count();

        //Check if in closed
		//t1 = std::chrono::high_resolution_clock::now();
		std::vector<Node*> nodesToAdd;
		std::vector<int> nodesToAdd_idx;
		for (unsigned int i=0; i<children.size(); i++) {
			Node *node = children[i];
			std::unordered_set<Node*, Hash, NodePointerEq>::const_iterator found = closed.find(node);
            if (found == closed.end()) {
                closed.insert(node);
                nodesToAdd.push_back(node);
                nodesToAdd_idx.push_back(i);
            } else if ((*found)->depth > node->depth) {
                (*found)->depth = node->depth;
                (*found)->parentMove = node->parentMove;
                (*found)->parent = node->parent;

                nodesToAdd.push_back(node);
                nodesToAdd_idx.push_back(i);
            } else {
                delete node;
            }
		}

        auto t2 = std::chrono::high_resolution_clock::now();
        tClosed += std::chrono::duration<double>(t2 - t1).count();

        // add to open
        for (unsigned int i=0; i<nodesToAdd.size(); i++) {
			Node *nodeToAdd = nodesToAdd[i];
			open.push(nodeToAdd);
		}

        auto t3 = std::chrono::high_resolution_clock::now();
        tPush += std::chrono::duration<double>(t3 - t2).count();

        searchItr++;

        //now = std::chrono::high_resolution_clock::now();
        //duration = now - search_start_time;
        //std::cout << "time: " << duration.count() <<" iterations: " << searchItr << " nodes generated: " << numNodesGenerated << std::endl;
    }
    std::vector<Move> buf;
    Node *currNode = solvedNode;
    while (currNode->depth > 0) {
        buf.push_back(static_cast<Move>(currNode->parentMove));
        currNode = currNode->parent;
    }
    std::vector<Move> solution;
    while(!buf.empty()){ // correctly arrange the turns for the solution
        Move m = buf.back();
        buf.pop_back();
        solution.push_back(m);
    }
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = now - search_start_time;
    std::cout << " time:" << duration.count() << " tExpand:" << tExpand << " tClosed:" << tClosed << " tPush:" << tPush 
    << " iterations: " << searchItr <<" nodes generated: " << numNodesGenerated
    << " nodes_per_second: " << double(duration.count() > 0 ?  numNodesGenerated / double(duration.count()) : 0.0)
    << " solution_length: " << int(solution.size()) << std::endl; 
    //std::cout << am.translate(am.reverse(solution)) << std::endl;
    return solution;
}

std::vector<Move> Solver::beam_search(const Puppet::State& start, const size_t width){
    auto start_time = std::chrono::high_resolution_clock::now();
    struct Node{ 
        Puppet::State state;
        Move last_turn;
        uint8_t value;
    };
    std::unordered_set<Puppet::State, StateHash, StateEq> closed;
    Puppet::State state = start;
    Node next;
    uint16_t len = 0;
    std::vector<Node> beam;
    std::vector<Node> kids;
    beam.reserve(width);
    kids.reserve(width * 9);
    beam.push_back({state, END, getHeuVal(start)});
    size_t nodes_gen = 1;
    size_t states_visited = 1;
    size_t max_nodes = 1;
    while(true){
        kids.clear();
        for(const Node& n: beam){
            uint8_t last_face = getFace(n.last_turn);
            state = n.state;
            uint16_t valid_moves = table.shape_moves[puppet.getShapeKey(state)];
            uint8_t move_idx;
            while((move_idx = __builtin_ctz(valid_moves)) < NUM_MOVES){
                valid_moves &= valid_moves - 1;
                Move m = static_cast<Move>(move_idx);
                if(getFace(m) == last_face) continue;
                next.state = state;
                puppet.turn(next.state, m);
                uint8_t newval = getHeuVal(next.state);
                states_visited++;
                if(newval == 0){
                    size_t size = kids.size();
                    if(size > max_nodes){
                        max_nodes = size;
                    }
                    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start_time;
                    std::cout << " time: " << duration.count() <<" nodes_generated: " << nodes_gen 
                    << " maximum_nodes_stored:"<< max_nodes
                    << " nodes/second: " << double(duration.count() > 0 ? nodes_gen / double(duration.count()) : 0.0)
                    << " states_visited:" << states_visited
                    << " solution_length: " << int(len + 1) << std::endl;
                    //std::cout << duration.count() << "," << nodes_gen << "," << nodes_gen << "," << int(len + 1) << "," << std::endl;
                    return {}; // no solution is returned
                }

                // check if state is in closed list
                auto it = closed.find(next.state);
                if(it == closed.end()){
                    next.value = newval;
                    next.last_turn = m;
                    kids.push_back(next);
                    closed.insert(next.state);
                    nodes_gen++;
                }   
            }
        }
        size_t size = kids.size();
        if(size > max_nodes){
            max_nodes = size;
        }
        if(kids.empty()){ // on lower beam sizes, the beam may run into dead ends
            std::cout << " NO SOLUTION FOUND!!!" << std::endl;
            return {};
        }
        size_t keep = std::min(width, kids.size());
        std::nth_element(kids.begin(), kids.begin() + keep - 1, kids.end(),
        [](const Node& a, const Node& b){ return a.value < b.value; }); // pick the best kids for the next beam
        kids.resize(keep);
        std::swap(beam, kids);
        len++;
    }
}

std::vector<Move> Solver::astar_weighted(const Puppet::State& start, const double weight){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    struct node{
        Puppet::State state;
        std::array<Move, 50> alg;
        uint8_t len;
        uint8_t value;
    };

    auto cmp = [weight](const node& a, const node& b){
        double fa = a.len * weight + a.value;
        double fb = b.len * weight + b.value;
        if(fa != fb) return fa > fb;
        return a.value > b.value;
    }; // generated by claude
    std::priority_queue<node, std::vector<node>, decltype(cmp)> found(cmp);
    std::unordered_map<Puppet::State, uint8_t, StateHash, StateEq> seen;
    size_t dup_total = 0;      

    node cur;
    node next;
    Puppet::State state = start;
    uint8_t last_face = 255;
    uint8_t newval = 0;
    found.push({state, {}, 0, getHeuVal(start)});
    size_t nodes_gen = 1;
    size_t max_nodes = 1;
    while(!found.empty()){
        if(found.size() > max_nodes){
            max_nodes = found.size();
        }
        cur = found.top();
        found.pop();
        state = cur.state;
        last_face = cur.len > 0 ? getFace(cur.alg[cur.len-1]) : 255;
        if(cur.len >= 50) continue;
        uint16_t info = table.shape_moves[puppet.getShapeKey(state)];
        uint8_t i;
        while((i = __builtin_ctz(info)) < NUM_MOVES){ // get idx of least significant 1
            Move m = static_cast<Move>(i);
            uint8_t face = getFace(m);
            if(face != last_face){
                next.state = state;
                puppet.turn(next.state, m);
                newval = getHeuVal(next.state);
                if(newval == 0){
                    now = std::chrono::high_resolution_clock::now();
                    duration = now - start_time;
                    std::vector<Move> alg;
                    for(uint8_t i = 0; i < cur.len; i++){
                        alg.push_back(cur.alg[i]);
                    }
                    alg.push_back(m);
                    //std::cout << "solution found. time: " << duration.count() <<" nodes generated: " << nodes_gen << " maximum nodes stored: "<< max_nodes 
                    //<< " nodes per second: " << double(duration.count() > 0 ? nodes_gen / double(duration.count()) : 0.0) << " states already seen: "<< dup_total << std::endl;
                    std::cout << am.translate(alg) << "  " << int(cur.len + 1) << std::endl;
                    std::cout << duration.count() << "," << nodes_gen << "," << max_nodes << "," << int(cur.len + 1) << "," << std::endl;
                    return alg;
                }
                uint8_t newlen = cur.len + 1;
                auto it = seen.find(next.state);
                if(it != seen.end()){
                    dup_total++;
                    if(it->second <= newlen){
                        info &= info - 1;
                        //continue;
                    } else {
                        it->second = newlen;
                    }
                } else {
                    seen.emplace(next.state, newlen);
                }
                next.alg = cur.alg;
                next.alg[cur.len] = m;
                next.len = cur.len + 1;
                next.value = newval;
                found.push(next);
                nodes_gen++;
            }
            info &= info - 1; // set least significant 1 to 0
        }
    }
    return {}; 
}

std::vector<Move> Solver::astar_parent_nodes(const Puppet::State& start){
    struct Node {
        Puppet::State state;
        uint32_t parent;
        Move move;
        uint8_t value;
    };
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<Node> arena;
    arena.reserve(1 << 24);
    arena.push_back({start, UINT32_MAX, {}, getHeuVal(start)});

    std::array<std::vector<uint32_t>, 3> found;
    found[0].push_back(0);
    uint8_t queue = 0;

    while(true){
        if(found[queue].empty()){ queue = (queue + 1) % 3; continue; }
        uint32_t cur_idx = found[queue].back();
        found[queue].pop_back();
        const Node cur = arena[cur_idx];
        uint8_t last_face = (cur.parent == UINT32_MAX) ? 255 : getFace(cur.move);

        uint16_t info = table.shape_moves[puppet.getShapeKey(cur.state)];
        uint8_t i;
        while((i = __builtin_ctz(info)) < NUM_MOVES){
            info &= info - 1;
            Move m = static_cast<Move>(i);
            if(getFace(m) != last_face){continue;}
            Puppet::State ns = cur.state;
            puppet.turn(ns, m);
            uint8_t newval = getHeuVal(ns);
            if(newval == 0){
                std::vector<Move> alg{m};
                for(uint32_t k = cur_idx; arena[k].parent != UINT32_MAX; k = arena[k].parent)
                    alg.push_back(arena[k].move);
                std::reverse(alg.begin(), alg.end());
                std::chrono::duration<double> d = std::chrono::high_resolution_clock::now() - start_time;
                //std::cout << "solution found. time: " << d.count() << std::endl;
                //std::cout << am.translate(am.reverse(alg)) << std::endl;
                std::cout << d.count() << "," << int(arena.size()) << "," << int(arena.size()) << "," << int(alg.size()) << "," << std::endl;
                return alg;
            }
            arena.push_back({ns, cur_idx, m, newval});
            found[(queue + 1 + newval - cur.value) % 3].push_back(arena.size() - 1);
        }
    }
}

std::vector<Move> Solver::iterative_deepening_rec(const Puppet::State& start, const size_t max_depth){
    const int h0 = getHeuVal(start);
    if(h0 == 0) return {};

    std::vector<Move> path(max_depth + 1);
    size_t path_len = 0;
    size_t nodes_gen = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    auto dfs = [&](auto& self, const Puppet::State& s, int g, int h, int bound,
                   uint8_t last_face, int& next_bound) -> bool {
        int f = g + h;
        if(f > bound){ next_bound = std::min(next_bound, f); return false; }
        if(g >= (int)max_depth) return false;

        uint16_t info = table.shape_moves[puppet.getShapeKey(s)];
        uint8_t i;
        while((i = __builtin_ctz(info)) < NUM_MOVES){
            info &= info - 1;
            Move m = static_cast<Move>(i);
            uint8_t face = getFace(m);
            if(face == last_face) continue;

            Puppet::State nxt = s;
            puppet.turn(nxt, m);
            int nh = getHeuVal(nxt);
            nodes_gen++;
            path[g] = m;
            if(nh == 0){ path_len = g + 1; return true; }
            if(self(self, nxt, g + 1, nh, bound, face, next_bound)) return true;
        }
        return false;
    };

    int bound = h0;
    while(bound <= (int)max_depth){
        int next_bound = INT_MAX;
        if(dfs(dfs, start, 0, h0, bound, 255, next_bound)){
            std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start_time;
            std::cout << duration.count() << "," << nodes_gen << "," << path_len << std::endl;
            return std::vector<Move>(path.begin(), path.begin() + path_len);
        }
        if(next_bound == INT_MAX) break;
        bound = next_bound;
    }
    return {};
}

std::vector<Move> Solver::random_state(const solvefunc& solve){
    // choose ranks at random
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> pickShape(0, NUM_SHAPES - 1);
    size_t shapeIdx = pickShape(gen);
    std::uniform_int_distribution<> pickCorners(0, NUM_CORNER_CYCLES - 1);
    size_t cornerIdx = pickCorners(gen);
    std::uniform_int_distribution<> pickEdges(0, NUM_EDGE_PERMUTATIONS - 1);
    size_t edgeIdx = pickEdges(gen);
    std::uniform_int_distribution<> pickFlips(0, NUM_EDGE_ORIENTATIONS - 1);
    size_t flipIdx = pickFlips(gen);
    size_t shapeKey;

    // get the shape key for the chosen rank
    for(shapeKey = 0; shapeKey < SHAPE_TABLE_SIZE; shapeKey++){
        if(table.shape_ranks[shapeKey] == shapeIdx) break;
    }

    //std::cout << shapeIdx <<" " << shapeHash <<" " << cornerIdx <<" "<< edgeIdx <<" "<< flipIdx << " " << std::bitset<8>(flipIdx)<< std::endl;
    Puppet::State start = puppet.stateFromRanks(shapeKey, cornerIdx, edgeIdx, flipIdx); // reconstruct the state
    //std::cout << std::endl;
    puppet.fixParity(start);  // check for odd parity
    return am.reverse(solve(start)); // the scramble is needed, not the solution
}

void Solver::scramble_set_random_states(const std::string& filename, size_t count, const solvefunc& solve, bool sort){
    std::vector<std::array<Move, 50>> scrambles; // scrambles are stored in arrays with length 50
    for(size_t i = 0; i < count; i++){
        std::vector<Move> scramble = random_state(solve);
        std::array<Move, 50> scramble_array;
        scramble_array.fill(END);
        uint8_t idx = 0;
        for(auto m : scramble){
            scramble_array[idx++] = m;
        }
        scrambles.push_back(scramble_array);
    }

    if (sort) {
        std::sort(scrambles.begin(), scrambles.end(),
            [](const auto& a, const auto& b) {
                auto length = [](const auto& scramble) {
                    return std::find(scramble.begin(), scramble.end(), END) - scramble.begin();
                };
                return length(a) < length(b);
            });
    }

    table.write_table<std::array<Move, 50>>(filename, scrambles);
}

void Solver::scramble_set_random_moves(const std::string& filename, size_t count, size_t depth, const solvefunc& solve, bool sort){
    std::vector<std::array<Move, 50>> scrambles;
    for(size_t i = 0; i < count; i++){
        Puppet::State state;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::array<uint8_t, 9> turns = {0,1,2,3,4,5,6,7,8};
        puppet.getSolved(state);
        std::uniform_int_distribution<> pick(0, 8);
        Move m = static_cast<Move>(turns[pick(gen)]);
        puppet.turn(state, m);
        size_t len = 0;
        while(len < depth){
            std::uniform_int_distribution<> pick(0, 8);
            m = static_cast<Move>(turns[pick(gen)]);
            uint16_t info = table.shape_moves[puppet.getShapeKey(state)];
            if((info >> static_cast<uint8_t>(m)) & 1u){
                puppet.turn(state, m);
                len++;
            }
        }
        std::vector<Move> scramble = am.reverse(solve(state));;
        std::array<Move, 50> scramble_array;
        scramble_array.fill(END);
        uint8_t idx = 0;
        for(auto m : scramble){
            scramble_array[idx++] = m;
        }
        scrambles.push_back(scramble_array);
    }

    if (sort) {
        std::sort(scrambles.begin(), scrambles.end(),
            [](const auto& a, const auto& b) {
                auto length = [](const auto& scramble) {
                    return std::find(scramble.begin(), scramble.end(), END) - scramble.begin();
                };
                return length(a) < length(b);
            });
    }

    table.write_table<std::array<Move, 50>>(filename, scrambles);
}

void Solver::scramble_set_decreasing_random_states(const std::string& filename, size_t max_depth, std::vector<size_t> count_per_depth, bool sort){
    std::vector<std::array<Move, 50>> scrambles;
    for(size_t depth = max_depth; depth > max_depth - count_per_depth.size(); depth--){
        while(count_per_depth[depth + count_per_depth.size() - max_depth - 1] > 0){
            // choose ranks at random
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> pickShape(0, NUM_SHAPES - 1);
            size_t shapeIdx = pickShape(gen);
            std::uniform_int_distribution<> pickCorners(0, NUM_CORNER_CYCLES - 1);
            size_t cornerIdx = pickCorners(gen);
            std::uniform_int_distribution<> pickEdges(0, NUM_EDGE_PERMUTATIONS - 1);
            size_t edgeIdx = pickEdges(gen);
            std::uniform_int_distribution<> pickFlips(0, NUM_EDGE_ORIENTATIONS - 1);
            size_t flipIdx = pickFlips(gen);
            size_t shapeHash;

            // get the shape key for the chosen rank
            for(shapeHash = 0; shapeHash < SHAPE_TABLE_SIZE; shapeHash++){
                if(table.shape_ranks[shapeHash] == shapeIdx) break;
            }
            Puppet::State start = puppet.stateFromRanks(shapeHash, cornerIdx, edgeIdx, flipIdx); // reconstruct the state
            puppet.fixParity(start); // check for odd parity

            // the max_depth parameter is used to skip long scrambles which are no longer needed for the set 
            std::vector<Move> scramble = am.reverse(iterative_deepening(start, depth));
            if(scramble.empty()) continue;

            std::array<Move, 50> scramble_array;
            scramble_array.fill(END);
            uint8_t length_found = 0;
            for(auto m : scramble){
                scramble_array[length_found++] = m;
            }
            int idx = length_found + count_per_depth.size() - max_depth - 1;
            if(idx<0) continue;
            if(count_per_depth[idx] > 0){
                count_per_depth[idx]--;
                scrambles.push_back(scramble_array);
                //std::cout << am.translate(scramble) << "  " << int(length_found) <<" "<< int(count_per_depth[idx])<< std::endl;
            }
        }
    }

    if (sort) {
        std::sort(scrambles.begin(), scrambles.end(),
            [](const auto& a, const auto& b) {
                auto length = [](const auto& scramble) {
                    return std::find(scramble.begin(), scramble.end(), END) - scramble.begin();
                };
                return length(a) < length(b);
            });
    }

    table.write_table<std::array<Move, 50>>(filename, scrambles);
}

void Solver::solve_scramble_set(const std::string& filename, const solvefunc& solve){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    std::vector<std::array<Move, 50>> scrambles;
    table.load_table<std::array<Move, 50>>(scrambles, filename);
    Puppet::State state;
    auto scount = 0; 
    for(auto s : scrambles){
        puppet.getSolved(state);
        auto scramblemoves = 0;
        for(auto m : s){
            if(m == END) break;
            scramblemoves++;
            puppet.turn(state, m);
        }
        std::cout << int(scount++) <<" scramble_length:"<<int(scramblemoves);
        solve(state);
    }
    now = std::chrono::high_resolution_clock::now();
    duration = now - start_time;
    std::cout << "endtime: " << duration.count() << std::endl;
}