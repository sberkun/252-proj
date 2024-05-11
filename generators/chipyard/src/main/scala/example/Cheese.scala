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
import freechips.rocketchip.rocket.RegFile
import scala.annotation.meta.param

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


  
  var N = 4 // number of cores; one nugget per core
  val nugget_acquire = Seq.fill(N)(Wire(new DecoupledIO(Bool())))
  val nugget_release = Seq.fill(N)(Wire(new DecoupledIO(Bool())))
  val nugget_value = Seq.fill(N)(RegInit(0.U(params.width.W)))
  val nugget_locked = Seq.fill(N)(RegInit(false.B))

  nugget_release.foreach(_.ready := true.B)
  nugget_acquire.foreach(_.valid := true.B)

  // conflict(i,j) means j is blocking i
  val nug_conflict = Seq.fill(N * N)(Wire(Bool()))
  val will_acq = Seq.fill(N)(Wire(Bool()))
  for (i <- 0 until N) {
    for (j <- 0 until N) {
      if (i == 0 && j == 0) {
        nug_conflict(i * N + j) := false.B
      } else if (i == j) {
        nug_conflict(i * N + j) := nug_conflict(i * N + j - 1)
      } else if (j == 0) {
        nug_conflict(i * N + j) := ((nugget_value(i) === nugget_value(j)) && nugget_locked(j))
      } else if (j < i) {
        nug_conflict(i * N + j) := nug_conflict(i * N + j - 1) || ((nugget_value(i) === nugget_value(j)) && nugget_locked(j))
      } else {
        nug_conflict(i * N + j) := nug_conflict(i * N + j - 1) || ((nugget_value(i) === nugget_value(j)) && (nugget_acquire(j).ready || nugget_locked(j))) 
      }
    }
    will_acq(i) := (!nug_conflict(i * N + N - 1)) && nugget_acquire(i).ready

    nugget_locked(i) := will_acq(i) || (nugget_locked(i) && !nugget_release(i).valid);
    nugget_acquire(i).bits := will_acq(i)
  }



  val queue_depth = 4

  val queue_pop = Wire(new DecoupledIO(UInt(params.width.W)))
  val queue_push = Wire(new DecoupledIO(UInt(params.width.W)))
  val queue_acquire = Wire(new DecoupledIO(Bool()))
  val queue_slots = RegInit(VecInit(Seq.fill(queue_depth){0.U(params.width.W)}))
  val queue_shift_n = Wire(Vec(queue_depth, Bool()))
  val queue_sem = RegInit(0.U(width = 3.W))
  
  queue_pop.valid := true.B
  queue_pop.bits := queue_slots(0)
  queue_acquire.valid := true.B
  queue_acquire.bits := queue_sem =/= queue_depth.U
  queue_push.ready := true.B

  //  - if queue_acquire.ready and sem != depth, sem goes up
  //  - if queue_pop.ready and queue_slots(0) != 0, sem goes down
  queue_sem := queue_sem + 
                (queue_acquire.ready && queue_sem =/= queue_depth.U) - 
                (queue_pop.ready && queue_slots(0) =/= 0.U)

  // shifts:    0       1     2      3
  // slots: [0] <- [1] <- [2] <- [3] <- (push or 0) 

  queue_shift_n(0) := queue_pop.ready || queue_slots(0) === 0.U
  for (i <- 1 until queue_depth) {
    queue_shift_n(i) := queue_shift_n(i-1) || queue_slots(i) === 0.U
    queue_slots(i-1) := Mux(queue_shift_n(i-1), queue_slots(i), queue_slots(i-1))
  }
  queue_slots(queue_depth-1) := Mux(queue_shift_n(queue_depth-1), 
      Mux(queue_push.valid, queue_push.bits, 0.U), queue_slots(queue_depth-1))


  regmap(
    0x00 -> Seq(RegField(1, lock_acquire(0), lock_release(0))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x04 -> Seq(RegField(1, lock_acquire(1), lock_release(1))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x08 -> Seq(RegField(1, lock_acquire(2), lock_release(2))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x0C -> Seq(RegField(1, lock_acquire(3), lock_release(3))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x10 -> Seq(RegField(1, lock_acquire(4), lock_release(4))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x14 -> Seq(RegField(1, lock_acquire(5), lock_release(5))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x18 -> Seq(RegField(1, lock_acquire(6), lock_release(6))), // sets lock_acquire.ready on read, lock_release.valid on write
    0x1C -> Seq(RegField(1, lock_acquire(7), lock_release(7))), // sets lock_acquire.ready on read, lock_release.valid on write

    0x20 -> Seq(RegField(params.width, queue_pop, queue_push)),
    0x24 -> Seq(RegField.r(1, queue_acquire)),

    0x40 -> Seq(RegField(1, nugget_acquire(0), nugget_release(0))),
    0x44 -> Seq(RegField(params.width, nugget_value(0))),
    0x48 -> Seq(RegField(1, nugget_acquire(1), nugget_release(1))),
    0x4C -> Seq(RegField(params.width, nugget_value(1))),
    0x50 -> Seq(RegField(1, nugget_acquire(2), nugget_release(2))),
    0x54 -> Seq(RegField(params.width, nugget_value(2))),
    0x58 -> Seq(RegField(1, nugget_acquire(3), nugget_release(3))),
    0x5C -> Seq(RegField(params.width, nugget_value(3))),
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
      val cheese = sbus { LazyModule(new CheeseTL(params, sbus.beatBytes)(p)) }
      sbus.coupleTo("cheese") { cheese.node := TLFragmenter(sbus.beatBytes, sbus.blockBytes) := _ }
    }
    case None => {}
  }
}

class WithCheese extends Config((site, here, up) => {
  case CheeseKey => Some(CheeseParams())
})
