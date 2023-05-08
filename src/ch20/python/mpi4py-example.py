#!/usr/bin/env python3

from mpi4py import MPI

comm = MPI.COMM_WORLD

world_size = comm.Get_size()

world_rank = comm.Get_rank()

processor_name = MPI.Get_processor_name()

if 0 == world_rank:
    print(f'Hello world from processor "{processor_name}" ' f'rank {world_rank} out of {world_size} processors')

    for other_rank in range(1, world_size):
        msg = f'Hello {other_rank} from {world_rank}!'

        comm.send(msg, other_rank, tag=0)

    for other_rank in range(1, world_size):
        msg = comm.recv(source=other_rank, tag=0, status=None)
        print(f'Message: {msg}')
else:
    msg = comm.recv(source=0, tag=0, status=None)

    print(f'Process {world_rank} ' f'received data: {msg}')

    msg = f'Process {world_rank} reporting for duty.'
    comm.send(msg, 0, tag=0)

MPI.Finalize()
