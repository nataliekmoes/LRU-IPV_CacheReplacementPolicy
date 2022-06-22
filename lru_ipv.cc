
#include "mem/cache/replacement_policies/lru_ipv.hh"
#include <cassert>
#include <memory>
#include "sim/core.hh"
#include "params/LRUIPVRP.hh"
#include "debug/LRUIPVDebug.hh"
#include "base/logging.hh"
#include "base/trace.hh"
#include <string>

const uint8_t ipv[] = {0, 0, 1, 0, 3, 0, 1, 2, 1, 0, 5, 1, 0, 0, 1, 11, 13};  // the IPV  used

LRUIPVRP::LRUIPVRP(const Params *p)
: BaseReplacementPolicy(p), count(0), num_sets(0), stack_instance(nullptr)
{
}

LRUIPVRP::LRUReplData::LRUReplData(uint8_t index, std::shared_ptr<std::vector<LRUReplData*>> stack_ptr)
: index(index), stack_ptr(stack_ptr)
{
}

void
LRUIPVRP::update(std::vector<LRUReplData*>* s, uint8_t old_idx, uint8_t new_idx)
const
{
	std::string str = "";
	for(auto & i : *s)
	{
		str += std::to_string(i->index );
		str += " ";
	}
	DPRINTF(LRUIPVDebug, "%s\n", str);


	// if data is being moved up, shift blocks down
	if(new_idx < old_idx)
	{
		for(auto & i : *s)
		{
			if(i->index >= new_idx && i->index < old_idx)
				i->index++;
		}
	}
	// if data is being moved down, shift blocks up
	if(new_idx > old_idx)
	{
		for(auto & i : *s)
		{
			if(i->index <= new_idx && i->index > old_idx)
				i->index--;
		}
	}

	str = "";
	for(auto & i : *s)
	{
		str += std::to_string(i->index );
		str += " ";
	}
	DPRINTF(LRUIPVDebug, "%s\n", str);
}

void
LRUIPVRP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const
{
	std::shared_ptr<LRUReplData> rData = std::static_pointer_cast<LRUReplData>(replacement_data);

	DPRINTF(LRUIPVDebug,"INVALIDATE:   index: %i\n", rData->index);

	update(rData->stack_ptr.get(), rData->index, 16); // shift blocks in recency stack

	rData->index = 16;	// set the index of the replacement data to 16
}

void
LRUIPVRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
	std::shared_ptr<LRUReplData> rData = std::static_pointer_cast<LRUReplData>(replacement_data);

	DPRINTF(LRUIPVDebug,"TOUCH:   initial index: %i   new index: %i\n", rData->index, ipv[rData->index]);

	update(rData->stack_ptr.get(), rData->index, ipv[rData->index]); // shift blocks in recency stack

	rData->index = ipv[rData->index];  // set index of replacement data to the corresponding IPV index
}

void
LRUIPVRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
	std::shared_ptr<LRUReplData> rData = std::static_pointer_cast<LRUReplData>(replacement_data);

	DPRINTF(LRUIPVDebug,"RESET:   initial index: %i\n", rData->index);

	update(rData->stack_ptr.get(), rData->index, 13);	// shift blocks in recency stack

	rData->index = 13;	// set index to the index where data is inserted
}

ReplaceableEntry*
LRUIPVRP::getVictim(const ReplacementCandidates& candidates) const
{
	// There must be at least one replacement candidate
	assert(candidates.size() > 0);

	// Visit all candidates to find victim
	ReplaceableEntry* victim = candidates[0];
	for (const auto& candidate : candidates)
	{
		// Update victim entry if necessary
		if (std::static_pointer_cast<LRUReplData>(candidate->replacementData)->index > 15)
		{
			victim = candidate;
		}
	}

	return victim;
}

std::shared_ptr<ReplacementData>
LRUIPVRP::instantiateEntry()
{
	if(count % 16 == 0)  // make a new recency stack for each set
	{
		num_sets++;
		stack_instance = new std::vector<LRUReplData*>;
	}

	// create new replacement data
	LRUReplData* rData = new LRUReplData(13, std::shared_ptr<std::vector<LRUReplData*>>(stack_instance));

	update(stack_instance, 16, 13); 	// shift blocks in recency stack
	stack_instance->push_back(rData);	// add new data to recency stack

	count++;

	DPRINTF(LRUIPVDebug, "count: %i   num_sets: %i\n", count, num_sets);

	return std::shared_ptr<ReplacementData>(rData);
}

LRUIPVRP*
LRUIPVRPParams::create()
{
	return new LRUIPVRP(this);
}
