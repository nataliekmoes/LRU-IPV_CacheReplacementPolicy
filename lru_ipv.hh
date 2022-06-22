
#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_LRU_IPV_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_LRU_IPV_HH__

#include "mem/cache/replacement_policies/base.hh"

struct LRUIPVRPParams;

class LRUIPVRP : public BaseReplacementPolicy
{
  public:
    /** LRUIPV-specific implementation of replacement data. */
    struct LRUReplData : ReplacementData
    {
        /** index of replacement data on recency stack */
        uint8_t index;

        /** pointer shared between replacement data members to the recency stack */
        std::shared_ptr<std::vector<LRUReplData*>> stack_ptr;

        /** Default Constructor */
        LRUReplData(uint8_t index, std::shared_ptr<std::vector<LRUReplData*>> stack_ptr);
    };

    /** shift replacement data up or down in the recency stack to accommodate moving a replacement data
     *  to a new position in the stack
     */
    void update(std::vector<LRUReplData*>* s, uint8_t old_pos, uint8_t new_pos) const;

    /** used in instantiateEntry() to map multiple replacement data to a recency stack */
    uint64_t count;

    uint64_t num_sets;

    /** instance of the recency stack used in instantiateEntry()  */
    std::vector<LRUReplData*>* stack_instance;

    /** Convenience typedef. */
    typedef LRUIPVRPParams Params;

    /**
     * Construct and initialize this replacement policy.
     */
    LRUIPVRP(const Params *p);

    /**
     * Destructor.
     */
    ~LRUIPVRP() {}

    /**
     * Invalidate replacement data to set it as the next probable victim.
     * Sets its last touch tick as the starting tick.
     *
     * @param replacement_data Replacement data to be invalidated.
     */
    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
                                                              const override;

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
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const
                                                                     override;

    /**
     * Instantiate a replacement data entry.
     *
     * @return A shared pointer to the new replacement data.
     */
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

#endif
