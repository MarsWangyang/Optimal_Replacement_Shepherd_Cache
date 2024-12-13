

#include "mem/cache/replacement_policies/sc_rp.hh"

#include <cassert>
#include <memory>

#include <iostream>
#include <iomanip>
#include <random>
#include "params/SCRP.hh"
#include "sim/cur_tick.hh"

namespace gem5
{

GEM5_DEPRECATED_NAMESPACE(ReplacementPolicy, replacement_policy);
namespace replacement_policy
{

SC::SC(const Params &p)
  : Base(p)
{
}

void
SC::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
{
    // Reset last touch timestamp
    // for(int i=0;i<std::static_pointer_cast<SCReplData>(replacement_data)->CM.size();++i){
    //     std::static_pointer_cast<SCReplData>(replacement_data)->CM[i] = Tick(0);
        
    // }
}

void
SC::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Update last touch timestamp
    for(int i = 0 ; i < std::static_pointer_cast<SCReplData>(replacement_data)->CM.size(); ++i){
        if(std::static_pointer_cast<SCReplData>(replacement_data)->CM[i] == 0){
            std::static_pointer_cast<SCReplData>(replacement_data)->CM[i] = curTick();
            //printf("[Update] %d = %d\n",i, std::static_pointer_cast<SCReplData>(replacement_data)->CM[i]);
        }
    }

}

void
SC::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    
}



ReplaceableEntry* 
SC::getVictim(const ReplacementCandidates& candidates) const
{
        // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    ReplaceableEntry* victim;

    // std::cout << "****************Begin*******************" <<std::endl;
    // for(int i = 0; i < candidates.size(); i++) {
    // auto data = std::static_pointer_cast<SCReplData>(candidates[i]->replacementData);
    
    // std::cout << std::setw(8) << "Index: " << std::setw(4) << i 
    //           << std::setw(15) << " Candidates: " << data.get()
    //           << std::setw(12) << " Counter0: " << std::setw(10) << data->CM[0]
    //           << std::setw(12) << " Counter1: " << std::setw(10) << data->CM[1]
    //           << std::setw(12) << " Counter2: " << std::setw(10) << data->CM[2]
    //           << std::setw(12) << " Counter3: " << std::setw(10) << data->CM[3]
    //           << std::setw(10) << " isLast: " << std::setw(4) << data->isLast
    //           << std::setw(12) << " isEmpty: " << std::setw(4) << data->isEmpty
    //           << std::setw(10) << " isSC1: " << std::setw(4) << data->isSC[0]
    //           << std::setw(10) << " isSC2: " << std::setw(4) << data->isSC[1]
    //           << std::setw(10) << " isSC2: " << std::setw(4) << data->isSC[2]
    //           << std::setw(10) << " isSC3: " << std::setw(4) << data->isSC[3]
    //           << std::endl;
    // }
    // std::cout << std::endl;

    int CMCol = 0;
    std::vector<int> idx_sc(4);
   
    int idx_isLast = 0;
    int scSize = candidates.size()/4;
    bool isFull = false;
    // Search the idx for sc1 sc2 and last
    for(int i=0;i<candidates.size();i++){
        for(int j=0;j<scSize;j++){
            if(std::static_pointer_cast<SCReplData>(candidates[i]->replacementData)->isSC[j]){
                idx_sc[j] = i;
                isFull = true;
            }
        }
        if(std::static_pointer_cast<SCReplData>(candidates[i]->replacementData)->isLast ){
            if(idx_isLast!=0){
                printf("%d %d\n",idx_isLast,i);
                assert(idx_isLast == 0);
            }
            idx_isLast = i;
        }
    }
    // Victim Eviction for Main Cache (if MC still has empty entries)
    if(!isFull){
        for (int i = 0; i < candidates.size(); i++) {
            // if is last empty set isSC 
            if(std::static_pointer_cast<SCReplData>(candidates[i]->replacementData)->isEmpty == true && (i==candidates.size()-1)){
                std::static_pointer_cast<SCReplData>(candidates[i]->replacementData)->isEmpty = false;
                for(int j=0;j<scSize;j++){
                    std::static_pointer_cast<SCReplData>(candidates[j+scSize*3]->replacementData)->isSC[j] = true;
                }
                std::static_pointer_cast<SCReplData>(candidates[candidates.size()-1]->replacementData)->isLast = true;
                // Once the cache full -> reset all of the CM to zero 
                for(int j=0;j<candidates.size();j++){
                    for(int k=0;k<scSize;++k){
                        std::static_pointer_cast<SCReplData>(candidates[j]->replacementData)->CM[k] = Tick(0);
                    }
                }
                for(int j=0;j<scSize-1;j++){
                    std::static_pointer_cast<SCReplData>(candidates[candidates.size()-1]->replacementData)->CM[j] = Tick(1);            
                }
                return victim = candidates[i];
            }
            if (std::static_pointer_cast<SCReplData>(candidates[i]->replacementData)->isEmpty == true) {
                std::static_pointer_cast<SCReplData>(candidates[i]->replacementData)->isEmpty = false;
                return victim = candidates[i];
            }
        }
    }
    
    

    // Choose which SC is evicted
    for(int i=0;i<scSize;i++){
        if (std::static_pointer_cast<SCReplData>(candidates[idx_sc[i]]->replacementData)->isLast == true){
            CMCol = i;
        }
    }    

    // Choose which cache is evicted (SC+MC) -> find the maximum CM[CMCol]
    victim = candidates[0];
    int victimIndex = 0;
    for (int i = 0; i < candidates.size(); i++) {
        if (std::static_pointer_cast<SCReplData>(candidates[i]->replacementData)->CM[CMCol]==0){
            victim = candidates[i];
            victimIndex = i;
            break;
        }
        if ((std::static_pointer_cast<SCReplData>(candidates[i]->replacementData)->CM[CMCol] > std::static_pointer_cast<SCReplData>(victim->replacementData)->CM[CMCol])){
                victim = candidates[i];
                victimIndex = i;
            }
    }

    // std::cout<<"SC Index:"<< idx_sc[0]<<" "<<idx_sc[1]<<" "<<idx_sc[2]<<" "<<idx_sc[3]<<" \n";

    // printf("[VICTIM] %d\n\n",victimIndex);

    for(int i=0;i<scSize;i++){
        if(std::static_pointer_cast<SCReplData>(candidates[idx_sc[i]]->replacementData)->isLast == true){
            std::static_pointer_cast<SCReplData>(candidates[idx_sc[i]]->replacementData)->isSC[i] = false;
            std::static_pointer_cast<SCReplData>(candidates[victimIndex]->replacementData)->isSC[i] = true;
        }
    }

    // Reset CM base on the new SC
    for(int i=0;i<candidates.size();++i){
        std::static_pointer_cast<SCReplData>(candidates[i]->replacementData)->CM[CMCol] = Tick(0);
    }

    for(int i=0;i<scSize;i++){
        if (std::static_pointer_cast<SCReplData>(candidates[idx_sc[i]]->replacementData)->isLast == true) {
            for(int j=0;j<scSize;j++){
                if(j!=i){
                    std::static_pointer_cast<SCReplData>(candidates[victimIndex]->replacementData)->CM[j] = Tick(1);
                }
            }
        }
    }
    // update SC FIFO switch isLast
    for(int i=0;i<scSize;i++){
        if(std::static_pointer_cast<SCReplData>(candidates[idx_sc[i]]->replacementData)->isLast == true && i==scSize-1){
            std::static_pointer_cast<SCReplData>(candidates[idx_sc[i]]->replacementData)->isLast = false;
            std::static_pointer_cast<SCReplData>(candidates[victimIndex]->replacementData)->isLast = false;
            std::static_pointer_cast<SCReplData>(candidates[idx_sc[0]]->replacementData)->isLast = true;
            break;
        }
        else if(std::static_pointer_cast<SCReplData>(candidates[idx_sc[i]]->replacementData)->isLast == true){
            std::static_pointer_cast<SCReplData>(candidates[idx_sc[i]]->replacementData)->isLast = false;
            std::static_pointer_cast<SCReplData>(candidates[victimIndex]->replacementData)->isLast = false;
            std::static_pointer_cast<SCReplData>(candidates[idx_sc[i+1]]->replacementData)->isLast = true;
            break;
        }
    }

    return victim;
}

std::shared_ptr<ReplacementData>
SC::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new SCReplData());
}

} // namespace replacement_policy
}


