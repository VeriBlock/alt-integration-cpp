#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_

#include <veriblock/entities/payloads.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/storage/endorsement_storage.hpp>

namespace altintegration {

class PopStorage : public EndorsementStorage<AltPayloads>,
                   public EndorsementStorage<VTB> {

};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_