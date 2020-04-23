#include <veriblock/blockchain/commands/addendorsement.hpp>

namespace altintegration {

bool AddVbkEndorsement::Execute(ValidationState& state) {

}

void AddVbkEndorsement::UnExecute() {

}

AddVbkEndorsement::AddVbkEndorsement(BlockIndex<AltBlock>& index,
                                     std::shared_ptr<VbkEndorsement> e)
    :  {}

}  // namespace altintegration