package chipyard.example

import chisel3._
import chisel3.util._
import chisel3.experimental.{IntParam, BaseModule}
import freechips.rocketchip.amba.axi4._
import freechips.rocketchip.subsystem.BaseSubsystem
import org.chipsalliance.cde.config.{Parameters, Field, Config}
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.regmapper.{HasRegMap, RegField}
import freechips.rocketchip.tilelink._
import freechips.rocketchip.util.UIntIsOneOf

case class CheeseParams(
  address: BigInt = 0x4000,
  width: Int = 32)

case object CheeseKey extends Field[Option[CheeseParams]](None)



trait CheeseTopIO extends Bundle {
  val cheese_busy = Output(Bool())
}



trait CheeseModule extends HasRegMap {
  val io: CheeseTopIO

  implicit val p: Parameters
  def params: CheeseParams
  val clock: Clock
  val reset: Reset

  io.cheese_busy := true.B


  val numLocks = 8 // Number of lock states

  // Create lists for lock_acquire, lock_release, and lock_state
  val lock_acquire = Seq.fill(numLocks)(Wire(new DecoupledIO(Bool())))
  val lock_release = Seq.fill(numLocks)(Wire(new DecoupledIO(Bool())))
  val lock_state = Seq.fill(numLocks)(RegInit(false.B))

  // Connect release always ready, and acquire always valid
  lock_release.foreach(_.ready := true.B)
  lock_acquire.foreach(_.valid := true.B)

  // Update lock_state and lock_acquire.bits for each lock
  for (i <- 0 until numLocks) {
    lock_acquire(i).bits := lock_state(i) || lock_release(i).valid
    lock_state(i) := !lock_acquire(i).ready && (lock_state(i) || lock_release(i).valid)
  }

  regmap(
    0x00 -> Seq(RegField(1, lock_acquire(0), lock_release(0))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x04 -> Seq(RegField(1, lock_acquire(1), lock_release(1))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x08 -> Seq(RegField(1, lock_acquire(2), lock_release(2))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x0C -> Seq(RegField(1, lock_acquire(3), lock_release(3))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x10 -> Seq(RegField(1, lock_acquire(4), lock_release(4))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x14 -> Seq(RegField(1, lock_acquire(5), lock_release(5))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x18 -> Seq(RegField(1, lock_acquire(6), lock_release(6))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x1C -> Seq(RegField(1, lock_acquire(7), lock_release(7))), // sets lock_acquire.ready on read, lock_release.valid on write
  )
}

class CheeseTL(params: CheeseParams, beatBytes: Int)(implicit p: Parameters)
  extends TLRegisterRouter(
    params.address, "cheese", Seq("ucbbar,cheese"),
    beatBytes = beatBytes)(
      new TLRegBundle(params, _) with CheeseTopIO)(
      new TLRegModule(params, _, _) with CheeseModule)


trait CanHavePeripheryCheese { this: BaseSubsystem =>
  p(CheeseKey) match {
    case Some(params) => {
      val cheese = pbus { LazyModule(new CheeseTL(params, pbus.beatBytes)(p)) }
      pbus.coupleTo("cheese") { cheese.node := TLFragmenter(pbus.beatBytes, pbus.blockBytes) := _ }
    }
    case None => {}
  }
}

class WithCheese extends Config((site, here, up) => {
  case CheeseKey => Some(CheeseParams())
})
