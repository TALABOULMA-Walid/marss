
// 
// Copyright 2009 Avadh Patel <apatel@cs.binghamton.edu>
//
// Authors:
//	Avadh Patel
//	Furat Afram
//

#ifndef CPU_CONTROLLER_H
#define CPU_CONTROLLER_H

#include <controller.h>
#include <interconnect.h>
#include <superstl.h>
#include <memoryStats.h>
//#include <logic.h>

namespace Memory {

struct CPUControllerQueueEntry : public FixStateListObject 
{
	MemoryRequest *request;
	int cycles;
	int depends;
	bool annuled;

	void init() {
		request = null;
		cycles = -1;
		depends = -1;
		annuled = false;
	}

	ostream& print(ostream& os) const {
		if(!request) {
			os << "Free Request Entry";
			return os;
		}
		os << "Request{", *request, "} ";
		os << "cycles[", cycles, "] ";
		os << "depends[", depends, "] ";
		os << endl;
		return os;
	}
};

static inline ostream& operator <<(ostream& os, 
		const CPUControllerQueueEntry& entry)
{
	return entry.print(os);
//	return os;
}

struct CPUControllerBufferEntry : public FixStateListObject
{
	W64 lineAddress;
	int idx;

	void reset(int i) {
		idx = i;
		lineAddress = -1;
	}

	void init() {}

	ostream& print(ostream& os) const {
		os << "lineAddress[", lineAddress, "] ";
		return os;
	}
};

static inline ostream& operator <<(ostream& os, 
		const CPUControllerBufferEntry& entry)
{
	entry.print(os);
	return os;
}

class CPUController : public Controller
{
	private:
		Interconnect *int_L1_i_;
		Interconnect *int_L1_d_;
		int icacheLineBits_;
		int dcacheLineBits_;
		CacheStats *stats_;
		CacheStats *totalStats_;

		FixStateList<CPUControllerQueueEntry, \
			CPU_CONT_PENDING_REQ_SIZE> pendingRequests_;
		FixStateList<CPUControllerBufferEntry, \
			CPU_CONT_ICACHE_BUF_SIZE> icacheBuffer_;

		bool is_icache_buffer_hit(MemoryRequest *request) ;

		CPUControllerQueueEntry* find_dependency(MemoryRequest *request);

		void wakeup_dependents(CPUControllerQueueEntry *queueEntry);

		void finalize_request(CPUControllerQueueEntry *queueEntry);

		CPUControllerQueueEntry* find_entry(MemoryRequest *request);

		W64 get_line_address(MemoryRequest *request) {
			if(request->is_instruction())
				return request->get_physical_address() >> icacheLineBits_;
			return request->get_physical_address() >> dcacheLineBits_;
		}

	public:
		CPUController(W8 coreid, const char *name, 
				MemoryHierarchy *memoryHierarchy);
		bool handle_request_cb(void *arg);
		bool handle_interconnect_cb(void *arg);
		int access_fast_path(Interconnect *interconnect,
				MemoryRequest *request);
		void clock();
		void register_interconnect_L1_d(Interconnect *interconnect);
		void register_interconnect_L1_i(Interconnect *interconnect);
		int access(MemoryRequest *request) {
			return access_fast_path(null, request);
		}
		void print(ostream& os) const;
		bool is_full(bool fromInterconnect = false) const {
			return pendingRequests_.isFull();
		}
		bool is_cache_availabe(bool is_icache);
		void annul_request(MemoryRequest *request);

		void print_map(ostream& os)
		{
			os << "CPU-Controller: ", get_name(), endl;
			os << "\tconnected to: ", endl;
			os << "\t\tL1-i: ", int_L1_i_->get_name(), endl;
			os << "\t\tL1-d: ", int_L1_d_->get_name(), endl;
		}

};

static inline ostream& operator <<(ostream& os, const CPUController& controller)
{
	controller.print(os);
	return os;
}

};

#endif // CPU_CONTROLLER_H
