#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_SC_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_SC_RP_HH__

#include "mem/cache/replacement_policies/base.hh"

namespace gem5
{

struct SCRPParams;

GEM5_DEPRECATED_NAMESPACE(ReplacementPolicy, replacement_policy);
namespace replacement_policy
{

class SC : public Base
{
  protected:
    /** LRU-specific implementation of replacement data. */
    struct SCReplData : ReplacementData
    {
        /** Tick on which the entry was last touched. */
        //Tick lastTouchTick;
        //Tick CM[2];
        std::vector<Tick> CM;
        bool isLast;
        bool isEmpty;
        std::vector<bool> isSC;
        /**
         * Default constructor. Invalidate data.
         */
        SCReplData() : CM(4, 0),isLast(false), isEmpty(true), isSC(4,false)  {}
    };

  public:
    typedef SCRPParams Params;
    SC(const Params &p);
    ~SC() = default;

    /**
     * Invalidate replacement data to set it as the next probable victim.
     * Sets its last touch tick as the starting tick.
     *
     * @param replacement_data Replacement data to be invalidated.
     */
    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
                                                                    override;

    /**
     * Touch an entry to update its replacement data.
     * Sets its last touch tick as the current tick.
     *
     * @param replacement_data Replacement data to be touched.
     */
    void touch(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Reset replacement data. Used when an entry is inserted.
     * Sets its last touch tick as the current tick.
     *
     * @param replacement_data Replacement data to be reset.
     */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Find replacement victim using LRU timestamps.
     *
     * @param candidates Replacement candidates, selected by indexing policy.
     * @return Replacement entry to be replaced.
     */
    //ReplaceableEntry* getVictim( ReplacementCandidates& candidates);
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const override;

    /**
     * Instantiate a replacement data entry.
     *
     * @return A shared pointer to the new replacement data.
     */
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

} // namespace replacement_policy
} // namespace gem5

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_LRU_RP_HH__
