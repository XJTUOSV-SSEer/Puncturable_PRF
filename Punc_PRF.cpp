#include "Punc_PRF.h"

GGMTree* PuncPRF::tree = nullptr;
uint8_t* PuncPRF::key = nullptr;
unordered_map<long, uint8_t*> PuncPRF::keys;
vector<GGMNode> PuncPRF::node_list;
vector<GGMNode> PuncPRF::remain_node;

void PuncPRF::SetUp(uint8_t* key, int key_len){
    PuncPRF::key = new uint8_t(key_len);
    memcpy(PuncPRF::key, key, key_len);
    tree = new GGMTree(GGM_SIZE);
}

void PuncPRF::Punc(vector<long> delete_pos){
    vector<long> pos;
    for (int i = 0; i < GGM_SIZE; ++i) {
        pos.emplace_back(i);
    }

    vector<long> remain_pos;
    set_difference(pos.begin(), pos.end(),
                   delete_pos.begin(), delete_pos.end(),
                   inserter(remain_pos, remain_pos.begin()));

    PuncPRF::node_list.reserve(remain_pos.size()); //preallocate memory for vector
    for (long pos : remain_pos) {
        PuncPRF::node_list.emplace_back(GGMNode(pos, tree->get_level()));
    }
    PuncPRF::remain_node = tree->min_coverage(PuncPRF::node_list);

    for(auto & i : PuncPRF::remain_node) {
        memcpy(i.key, PuncPRF::key, AES_BLOCK_SIZE);
        // for(int j=0;j<16;j++){
        //     printf ( "%02x ", i.key[j]);
        // }
        // cout<<endl;
        GGMTree::derive_key_from_tree(i.key, i.index, i.level, 0);
        // for(int j=0;j<16;j++){
        //     printf ( "%02x ", i.key[j]);
        // }
        // cout<<endl;
    }
}

void PuncPRF::PPRF_compute_all_keys() {
    PuncPRF::keys.clear();
    int level = tree->get_level();
    for(GGMNode node : PuncPRF::remain_node) {
        for (int i = 0; i < pow(2, level - node.level); ++i) {
            int offset = ((node.index) << (level - node.level)) + i;
            uint8_t derive_key[AES_BLOCK_SIZE];
            memcpy(derive_key, node.key, AES_BLOCK_SIZE);
            // for(int i=0;i<16;i++){
            //     printf ( "%02x ",node.key[i]);
            // }
            GGMTree::derive_key_from_tree(derive_key, offset, level - node.level, 0);
            if(PuncPRF::keys.find(offset) == PuncPRF::keys.end()) {
                PuncPRF::keys[offset] = (uint8_t*) malloc(AES_BLOCK_SIZE);
                memcpy(PuncPRF::keys[offset], derive_key, AES_BLOCK_SIZE);
//                for (int j = 0 ; j <16; ++j){
//                    printf ( "%02x ",keys[offset][j]);
//                }
//                cout << endl;
            }
        }
    }
}

void PuncPRF::PPRF_Eval(long msg ){
    if(PuncPRF::keys.find(msg) == PuncPRF::keys.end()){
        cout << "stop at punctured point !" << endl;
    }
    else{
        cout <<"Original pseudo random value of "<< msg << " is : " << endl;
        for (int j = 0 ; j <16; ++j){
            printf ( "%02x ",PuncPRF::keys[msg][j]);
        }
        cout << endl ;
    }
}