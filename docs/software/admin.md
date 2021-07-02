---
title: Administration
---

## Introduction

DigitalBits Core is the program nodes use to communicate with other nodes to create and maintain the DigitalBits peer-to-peer network.  It's an implementation of the DigitalBits Consensus Protocol configured to construct a chain of ledgers guaranteed to be in agreement across all participating nodes at all times.

This document describes various aspects of installing, configuring, and maintaining a `digitalbits-core` node.  It will explain:


  - [why you should run a node](#why-run-a-node)
  - [what you need to set up](#instance-setup)
  - [how to install digitalbits core](#installing)
  - [how to configure your node](#configuring)
  - [how quorum sets work](#choosing-your-quorum-set)
  - [how to prepare your environment before the first run](#environment-preparation)
  - [how to join the network](#joining-the-network)
  - [how logging works](#logging)
  - [node monitoring and diagnostics](#monitoring-and-diagnostics)
  - [how to perform validator maintenance](#validator-maintenance)
  - [how to perform network wide updates](#network-configuration)

## Why run a node?

### Benefits of running a node

You get to run your own Frontier instance:

  * Allows for customizations (triggers, etc) of the business logic or APIs
  * Full control of which data to retain (historical or online)
  * A trusted entry point to the network
    * Trusted end to end (can implement additional counter measures to secure services)
    * Open Frontier increases customer trust by allowing to query at the source (ie: larger token issuers have an official endpoint that can be queried)
  * Control of SLA

note: in this document we use "Frontier" as the example implementation of a first tier service built on top of digitalbits-core, but any other system would get the same benefits.

### Level of participation to the network

As a node operator you can participate to the network in multiple ways.

|                                                    | watcher       | archiver                            | basic validator                                                                                                             | full validator                       |
| -------------------------------------------------- | ------------- | ----------------------------------- | --------------------------------------------------------------------------------------------------------------------------- | ------------------------------------ |
| description                                        | non-validator | all of watcher + publish to archive | all of watcher + active participation in consensus (submit proposals for the transaction set to include in the next ledger) | basic validator + publish to archive |
| submits transactions                               | yes           | yes                                 | yes                                                                                                                         | yes                                  |
| supports frontier                                   | yes           | yes                                 | yes                                                                                                                         | yes                                  |
| participates in consensus                          | no            | no                                  | yes                                                                                                                         | yes                                  |
| helps other nodes to catch up and join the network | no            | yes                                 | no                                                                                                                          | yes                                  |
| Increase the resiliency of the network             | No            | Medium                              | Low                                                                                                                         | High                                 |

From an operational point of view "watchers" and "basic validators" are about the
 same (they both compute an up to date version of the ledger).
"Archivers" or "Full validators" publish into an history archive which
 has additional cost.

#### Watcher nodes

Watcher nodes are configured to watch the activity from the network

Use cases:

  * Ephemeral instances, where having other nodes depend on those nodes is not desired
  * Potentially reduced administration cost (no or reduced SLA)
  * Real time network monitoring (which validators are present, etc)
  * Generate network meta-data for other systems (Frontier)

**Operational requirements**:

  * a [database](#database)

#### Archiver nodes

The purpose of Archiver nodes is to record the activity of the network in long term storage (AWS, Azure, etc).

[History Archives](#history-archives) contain snapshots of the ledger, all transactions and their results.

Use cases:

  * Everything that a watcher node can do
  * Need for a low cost compliance story
  * Participate to the network’s resiliency
  * Analysis of historical data

**Operational requirements**:

  * requires an additional internet facing blob store
  * a [database](#database)

#### Basic validators
Nodes configured to actively vote on the network.

Use cases:

  * Everything that a watcher node can do
  * Increase the network reliability
  * Enables deeper integrations by clients and business partners
  * Official endorsement of specific ledgers in real time (via signatures)
  * Quorum Set aligned with business priorities
  * Additional checks/invariants enabled
    * Validator can halt and/or signal that for example (in the case of an issuer) that it does not agree to something.

**Operational requirements**: 

  * secret key management (used for signing messages on the network)
  * a [database](#database)

#### Full validators

Nodes fully participating in the network.

Full validators are the true measure of how decentralized and redundant the network is as they are the only type of validators that perform all functions on the network.

Use cases:

  * All other use cases
  * Some full validators required to be v-blocking (~ N full validators, M other validators on the network -> require at least M+1 threshold)
  * Branding - strongest association with the network
  * Mutually beneficial - best way to support the network’s health and resilience

**Operational requirements**:

  * requires an additional internet facing blob store
  * secret key management (used for signing messages on the network)
  * a [database](#database)

## Instance setup
Regardless of how you install digitalbits-core (apt, source, docker, etc), you will need to configure the instance hosting it roughly the same way.

### Compute requirements
CPU, RAM, Disk and network depends on network activity. If you decide to collocate certain workloads, you will need to take this into account.

As of early 2018, digitalbits-core with PostgreSQL running on the same machine has no problem running on a [m5.large](https://aws.amazon.com/ec2/instance-types/m5/) in AWS (dual core 2.5 GHz Intel Xeon, 8 GB RAM).

Storage wise, 20 GB seems to be an excellent working set as it leaves plenty of room for growth.

### Network access

#### Interaction with the peer to peer network

  * **inbound**: digitalbits-core needs to allow all ips to connect to its `PEER_PORT` (default 11625) over TCP.
  * **outbound**: digitalbits-core needs access to connect to other peers on the internet on `PEER_PORT` (most use the default as well) over TCP.

#### Interaction with other internal systems

  * **outbound**:
    * digitalbits-core needs access to a database (postgresql for example), which may reside on a different machine on the network
    * other connections can safely be blocked
  * **inbound**: digitalbits-core exposes an *unauthenticated* HTTP endpoint on port `HTTP_PORT` (default 11626)
    * it is used by other systems (such as Frontier) to submit transactions (so may have to be exposed to the rest of your internal ips)
    *  query information (info, metrics, ...) for humans and automation
    *  perform administrative commands (schedule upgrades, change log levels, ...)

Note on exposing the HTTP endpoint:
if you need to expose this endpoint to other hosts in your local network, it is recommended to use an intermediate reverse proxy server to implement authentication. Don't expose the HTTP endpoint to the raw and cruel open internet.

## Installing

### Release version

In general you should aim to run the latest [release](https://github.com/xdbfoundation/DigitalBits/releases) as builds are backward compatible and are cumulative.

The version number scheme that we follow is `protocol_version.release_number.patch_number`, where

  * `protocol_version` is the maximum protocol version supported by that release (all versions are 100% backward compatible),
  * `release_number` is bumped when a set of new features or bug fixes not impacting the protocol are included in the release,
  * `patch_number` is used when a critical fix has to be deployed

### Installing from source
See the [INSTALL](https://github.com/xdbfoundation/DigitalBits/blob/master/INSTALL.md) for build instructions.

### Package based Installation
If you are using Ubuntu 20.04 LTS we provide the latest stable releases of [digitalbits-core](https://github.com/xdbfoundation/DigitalBits) and [frontier](https://github.com/xdbfoundation/go/tree/master/services/frontier) in Debian binary package format.

See [detailed installation instructions](https://github.com/xdbfoundation/DigitalBits/releases)

### Container based installation
Docker images are maintained in a few places, good starting points are:

   * the [quickstart image](https://github.com/xdbfoundation/quickstart)

## Configuring

Before attempting to configure digitalbits-core, it is highly recommended to first try running a private network or joining the test network. 

### Configuration basics
All configuration for digitalbits-core is done with a TOML file. By default 
digitalbits-core loads `./digitalbits-core.cfg`, but you can specify a different file to load on the command line:

`$ digitalbits-core --conf betterfile.cfg <COMMAND>`

The [example config](https://github.com/xdbfoundation/DigitalBits/blob/master/docs/digitalbits-core_example.cfg) is not a real configuration, but documents all possible configuration elements as well as their default values.

Here is an [example test network config](https://github.com/xdbfoundation/DigitalBits/blob/master/docs/digitalbits-core_testnet.cfg) for connecting to the test network.

Here is an [example public network config](hhttps://github.com/xdbfoundation/DigitalBits/blob/master/docs/digitalbits-core.cfg) for connecting to the public network.

The examples in this file don't specify `--conf betterfile.cfg` for brevity.

Auditing of the P2P network is enabled by default, see the [overlay topology](#overlay-topology-survey) section for more detail if you'd like to disable it

### Validating node
Nodes are considered **validating** if they take part in DCP and sign messages 
pledging that the network agreed to a particular transaction set.

If you want to validate, you must generate a public/private key for your node.
 Nodes shouldn't share keys. You should carefully *secure your private key*. 
If it is compromised, someone can send false messages to the network and those 
messages will look like they came from you.

Generate a key pair like this:

`$ digitalbits-core gen-seed`

the output will look something like

```
Secret seed: SBWJ7VAPK6ABBHIJSJMUAUTVR2LQGNZNWW7NQIRV5D2BSGCE3PZA5EFI
Public: GA7AIALXFIW2HN53Z77ZCYJIAW23NKGDMMU3ROYG3YQYLYV4ATCDL3FV

```

Place the seed in your config:

`NODE_SEED="SBWJ7VAPK6ABBHIJSJMUAUTVR2LQGNZNWW7NQIRV5D2BSGCE3PZA5EFI"`

and set the following value in your config:

`NODE_IS_VALIDATOR=true`

If you don't include a `NODE_SEED` or set `NODE_IS_VALIDATOR=true`, you will still
watch SCP and see all the data in the network but will not send validation messages.

NB: if you run more than one node, set the `HOME_DOMAIN` common to those nodes using the `NODE_HOME_DOMAIN` property.
Doing so will allow your nodes to be grouped correctly during [quorum set generation](#home-domains-array).

If you want other validators to add your node to their quorum sets, you should also share your public key (GDMTUTQ... ) by publishing a digitalbits.toml file on your homedomain.

### Choosing your quorum set
A good quorum set:

* aligns with your organization’s priorities 
* has enough redundancy to handle arbitrary node failures
* maintains good quorum intersection 

Since crafting a good quorum set is a difficult thing to do, digitalbits core *automatically* generates a quorum set for you based on structured information you provide in your config file.  You choose the validators you want to trust; digitalbits core configures them into an optimal quorum set.

To generate a quorum set, digitalbits core:
* Groups validators run by the same organization into a subquorum
* Sets the threshold for each of those subquorums
* Gives weights to those subquorums based on quality

While this does not absolve you of all responsibility — you still need to pick trustworthy validators and keep an eye on them to ensure that they’re consistent and reliable — it does make your life easier, and reduces the chances for human error.

#### Validator discovery
When you add a validating node to your quorum set, it’s generally because you trust the *organization* running the node: you trust XDB Foundation, not some anonymous DigitalBits public key. 

In order to create a self-verified link between a node and the organization that runs it, a validator declares a home domain on-chain using a `set_options` operation, and publishes organizational information in a digitalbits.toml file hosted on that domain.  

As a result of that link, you can look up a node by its DigitalBits public key and check the digitalbits.toml to find out who runs it. If you decide to trust an organization, you can use that list to collect the information necessary to add their nodes to your configuration.  

When you look at that list, you will discover that the most reliable organizations actually run more than one validator, and adding all of an organization’s nodes to your quorum set creates the redundancy necessary to sustain arbitrary node failure.  When an organization with a trio of nodes takes one down for maintenance, for instance, the remaining two vote on the organization’s behalf, and the organization’s network presence persists.

One important thing to note: you need to either depend on exactly one entity OR have **at least 4 entities** for automatic quorum set configuration to work properly.  At least 4 is the better option.

#### Home domains array
To create your quorum set, digitalbits cores relies on two arrays of tables: `[[HOME_DOMAINS]]` and `[[VALIDATORS]]`.  Check out the [example config](https://github.com/xdbfoundation/DigitalBits/blob/master/docs/digitalbits-core_example.cfg#L372) to see those arrays in action.

`[[HOME_DOMAINS]]` defines a superset of validators: when you add nodes hosted by the same organization to your configuration, they share a home domain, and the information in the `[[HOME_DOMAINS]]` table, specifically the quality rating, will automatically apply to every one of those validators. 

For each organization you want to add, create a separate `[[HOME_DOMAINS]]` table, and complete the following required fields:

Field | Requirements | Description
------|--------------|------------
HOME_DOMAIN | string | URL of home domain linked to a group of validators
QUALITY | string | Rating for organization's nodes: `HIGH`, `MEDIUM`, or `LOW`

Here’s an example:
```
[[HOME_DOMAINS]]
HOME_DOMAIN="testnet.digitalbits.io"
QUALITY="HIGH"

[[HOME_DOMAINS]]
HOME_DOMAIN="some-other-domain"
QUALITY="LOW"
```

#### Validators array
For each node you would like to add to your quorum set, complete a `[[VALIDATORS]]` table with the following fields:  

Field | Requirements | Description
------|--------------|------------
NAME | string | A unique alias for the node
QUALITY | string | Rating for node (required unless specified in `[[HOME_DOMAINS]]`): `HIGH`, `MEDIUM`, or `LOW`.
HOME_DOMAIN | string | URL of home domain linked to validator
PUBLIC_KEY | string | DigitalBits public key associated with validator
ADDRESS | string | Peer:port associated with validator (optional)
HISTORY | string | archive GET command associated with validator (optional)

If the node's `HOME_DOMAIN` aligns with an organization defined in the `[[HOME_DOMAINS]]` array, the quality rating specified there will apply to the node.  If you’re adding an individual node that is *not* covered in that array, you’ll need to specify the `QUALITY` here.

Here’s an example:

```
[[VALIDATORS]]
NAME="deu-1"
HOME_DOMAIN="testnet.digitalbits.io"
PUBLIC_KEY="GCV5THURTQOWMLBB6QSL5CJJTQHTZN5GTZ2QGJCDOLLP4JZSK56SHNIV"
ADDRESS="deu-1.testnet.digitalbits.io"
HISTORY="curl -sf https://history.testnet.digitalbits.io/node1/{0} -o {1}"

[[VALIDATORS]]
NAME="deu-2"
HOME_DOMAIN="testnet.digitalbits.io"
PUBLIC_KEY="GCVJL3CPBWAJMYTR7PAOKNQMZ6KWDZUZNY4P6ACKACQETXPK3XOU3YUI"
ADDRESS="deu-2.testnet.digitalbits.io"
HISTORY="curl -sf https://history.testnet.digitalbits.io/node2/{0} -o {1}"

[[VALIDATORS]]
NAME="deu-3"
HOME_DOMAIN="testnet.digitalbits.io"
PUBLIC_KEY="GD4WG4HSA7CODZBSCXOPGVZM2RZ4BEEYH36WJ4PTTV4C474DZ5QL4LX7"
ADDRESS="deu-3.testnet.digitalbits.io"
HISTORY="curl -sf https://history.testnet.digitalbits.io/node3/{0} -o {1}"

[[VALIDATORS]]
NAME="some-random-node"
QUALITY="LOW"
HOME_DOMAIN="somerandomdomain.com"
PUBLIC_KEY="GA7AIALXFIW2HN53Z77ZCYJIAW23NKGDMMU3ROYG3YQYLYV4ATCDL3FV"
ADDRESS="node.somerandomdomain.com"
```

#### Validator quality
`QUALITY` is a required field for each node you add to your quorum set.  Whether you specify it for a suite of nodes in `[[HOME_DOMAINS]]` or for a single node in `[[VALIDATORS]]`, it means the same thing, and you have the same three rating options: HIGH, MEDIUM, or LOW.

**HIGH** quality validators are given the most weight in automatic quorum set configuration.  Before assigning a high quality rating to a node, make sure it has low latency and good uptime, and that the organization running the node is reliable and trustworthy.  

A high quality a validator:

* publishes an archive
* belongs to a suite of nodes that provide redundancy 

Choosing redundant nodes is good practice.  The archive requirement is programmatically enforced.

**MEDIUM** quality validators are nested below high quality validators, and their combined weight is equivalent to a *single high quality entity*.  If a node doesn't publish an archive, but you deem it reliable, or have an organizational interest in including in your quorum set, give it a medium quality rating.  

**LOW** quality validators are nested below medium quality validators, and their combined weight is equivalent to a *single medium quality entity*.    Should they prove reliable over time, you can upgrade their rating to medium to give them a bigger role in your quorum set configuration. 
 
#### Automatic quorum set generation
Once you add validators to your configuration, digitalbits core automatically generates a quorum set using the following rules:

* Validators with the same home domain are automatically grouped together and given a threshold requiring a simple majority (2f+1)
* Heterogeneous groups of validators are given a threshold assuming byzantine failure (3f+1)
* Entities are grouped by QUALITY and nested from HIGH to LOW 
* HIGH quality entities are at the top, and are given decision-making priority 
* The combined weight of MEDIUM quality entities equals a single HIGH quality entity  
* The combined weight of LOW quality entities equals a single MEDIUM quality entity


#### Quorum and overlay network

It is generally a good idea to give information to your validator on other validators that you rely on. This is achieved by configuring `KNOWN_PEERS` and `PREFERRED_PEERS` with the addresses of your dependencies.

Additionally, configuring `PREFERRED_PEER_KEYS` with the keys from your quorum set might be a good idea to give priority to the nodes that allows you to reach consensus.

Without those settings, your validator depends on other nodes on the network to forward you the right messages, which is typically done as a best effort.

#### Special considerations during quorum set updates

Sometimes an organization needs to make changes that impact other's quorum sets:

  * taking a validator down for long period of time
  * adding new validators to their pool

In both cases, it's crucial to stage the changes to preserve quorum intersection and general good health of the network:

  * removing too many nodes from your quorum set *before* the nodes are taken down : if different people remove different sets the remaining sets may not overlap between nodes and may cause network splits
  * adding too many nodes in your quorum set at the same time : if not done carefully can cause those nodes to overpower your configuration

Recommended steps are for the entity that adds/removes nodes to do so first between their own nodes, and then have people reflect those changes gradually (over several rounds) in their quorum configuration.

## Environment preparation

### digitalbits-core configuration
Cross reference your validator settings, in particular:

  * environment specific settings
    * network passphrase
    * known peers
  * home domains and validators arrays
  * seed defined if validating
  * [Automatic maintenance](#cursors-and-automatic-maintenance) configured properly, especially when digitalbits-core is used in conjunction with a downstream system like Frontier.

### Database and local state

After configuring your [database](#database) and [buckets](#buckets) settings, when running digitalbits-core for the first time, you must initialize the database:

`$ digitalbits-core new-db`

This command will initialize the database as well as the bucket directory and then exit. 

You can also use this command if your DB gets corrupted and you want to restart it from scratch. 

#### Database
DigitalBits-core stores the state of the ledger in a SQL database.

This DB should either be a SQLite database or, for larger production instances, a separate PostgreSQL server.

*Note: Frontier currently depends on using PostgreSQL.*

For how to specify the database, 
see the [example config](https://github.com/xdbfoundation/DigitalBits/blob/master/docs/digitalbits-core_example.cfg).

##### Cursors and automatic maintenance

Some tables in the database act as a publishing queue for external systems such as Frontier and generate **meta data** for changes happening to the distributed ledger.

If not managed properly those tables will grow without bounds. To avoid this, a built-in scheduler will delete data from old ledgers that are not used anymore by other parts of the system (external systems included).

The settings that control the automatic maintenance behavior are: `AUTOMATIC_MAINTENANCE_PERIOD`,  `AUTOMATIC_MAINTENANCE_COUNT` and `KNOWN_CURSORS`.

By default, digitalbits-core will perform this automatic maintenance, so be sure to disable it until you have done the appropriate data ingestion in downstream systems (Frontier for example sometimes needs to reingest data).

If you need to regenerate the meta data, the simplest way is to replay ledgers for the range you're interested in after (optionally) clearing the database with `newdb`.

##### Meta data snapshots and restoration

Some deployments of digitalbits-core and Frontier will want to retain meta data for the _entire history_ of the network. This meta data can be quite large and computationally expensive to regenerate anew by replaying ledgers in digitalbits-core from an empty initial database state, as described in the previous section.

This can be especially costly if run more than once. For instance, when bringing a new node online. Or even if running a single node with Frontier, having already ingested the meta data _once_: a subsequent version of Frontier may have a schema change that entails re-ingesting it _again_.

Some operators therefore prefer to shut down their digitalbits-core (and/or Frontier) processes and _take filesystem-level snapshots_ or _database-level dumps_ of the contents of digitalbits-core's database and bucket directory, and/or Frontier's database, after meta data generation has occurred the first time. Such snapshots can then be restored, putting digitalbits-core and/or Frontier in a state containing meta data without performing full replay.

Any reasonably-recent state will do -- if such a snapshot is a little old, digitalbits-core will replay ledgers from whenever the snapshot was taken to the current network state anyways -- but this procedure can greatly accelerate restoring validator nodes, or cloning them to create new ones.


#### Buckets
DigitalBits-core stores a duplicate copy of the ledger in the form of flat XDR files 
called "buckets." These files are placed in a directory specified in the config 
file as `BUCKET_DIR_PATH`, which defaults to `buckets`. The bucket files are used
 for hashing and transmission of ledger differences to history archives. 

Buckets should be stored on a fast local disk with sufficient space to store several times the size of the current ledger. 
 
 For the most part, the contents of both directories can be ignored as they are managed by digitalbits-core.

### History archives
DigitalBits-core normally interacts with one or more "history archives," which are 
configurable facilities for storing and retrieving flat files containing history 
checkpoints: bucket files and history logs. History archives are usually off-site 
commodity storage services such as Amazon S3, Google Cloud Storage, 
Azure Blob Storage, or custom SCP/SFTP/HTTP servers. 

Use command templates in the config file to give the specifics of which 
services you will use and how to access them. 
The [example config](https://github.com/xdbfoundation/DigitalBits/blob/master/docs/digitalbits-core_example.cfg) 
shows how to configure a history archive through command templates. 

While it is possible to run a digitalbits-core node with no configured history 
archives, it will be _severely limited_, unable to participate fully in a 
network, and likely unable to acquire synchronization at all. At the very 
least, if you are joining an existing network in a read-only capacity, you 
will still need to configure a `get` command to access that network's history 
archives.

#### Configuring to get data from an archive

You can configure any number of archives to download from: digitalbits-core will automatically round-robin between them.

At a minimum you should configure `get` archives for each full validator referenced from your quorum set (see the `HISTORY` field in [validators array](#validators-array) for more detail).

Note: if you notice a lot of errors related to downloading archives, you should check that all archives in your configuration are up to date.


#### Configuring to publish to an archive
Archive sections can also be configured with `put` and `mkdir` commands to
 cause the instance to publish to that archive (for nodes configured as [archiver nodes](#archiver-nodes) or [full validators](#full-validators)).

The very first time you want to use your archive *before starting your node* you need to initialize it with:

`$ digitalbits-core new-hist <historyarchive>`

**IMPORTANT:**

   * make sure that you configure both `put` and `mkdir` if `put` doesn't automatically create sub-folders
   * writing to the same archive from different nodes is not supported and will result in undefined behavior, *potentially data loss*.
   * do not run `newhist` on an existing archive unless you want to erase it.

### Other preparation

In addition, your should ensure that your operating environment is also functional.

In no particular order:

  * logging and log rotation
  * monitoring and alerting infrastructure

## Starting your node

After having configured your node and its environment, you're ready to start digitalbits-core.

This can be done with a command equivalent to

`$ digitalbits-core run`

At this point you're ready to observe core's activity as it joins the network.

Review the [logging](#logging) section to get yourself familiar with the output of digitalbits-core.

### Interacting with your instance
While running, interaction with digitalbits-core is done via an administrative 
HTTP endpoint. Commands can be submitted using command-line HTTP tools such 
as `curl`, or by running a command such as

`$ digitalbits-core http-command <http-command>`

The endpoint is [not intended to be exposed to the public internet](#interaction-with-other-internal-systems). It's typically accessed by administrators, or by a mid-tier application to submit transactions to the DigitalBits network. 

See [commands](./commands.md) for a description of the available commands.

### Joining the network

You can review the section on [general node information](#general-node-information);

the node will go through the following phases as it joins the network:

#### Establish connection to other peers

You should see `authenticated_count` increase.

```json
"peers" : {
         "authenticated_count" : 3,
         "pending_count" : 4
      },
```

#### Observing consensus

Until the node sees a quorum, it will say
```json
"state" : "Joining DCP"
```

After observing consensus, a new field `quorum` will be set with information on what the network decided on, at this point the node will switch to "*Catching up*":

```json
      "quorum" : {
         "qset" : {
            "ledger" : 22267866,
            "cost" : 20883268,
            "agree" : 5,
            "delayed" : 0,
            "disagree" : 0,
            "fail_at" : 3,
            "hash" : "980a24",
            "lag_ms" : 430,
            "missing" : 0,
            "phase" : "EXTERNALIZE"
         },
         "transitive" : {
            "intersection" : true,
            "last_check_ledger" : 22267866,
            "node_count" : 21
         }
      },
      "state" : "Catching up",
```

#### Catching up

This is a phase where the node downloads data from archives.
The state will start with something like

```json
      "state" : "Catching up",
      "status" : [ "Catching up: Awaiting checkpoint (ETA: 35 seconds)" ]
```

and then go through the various phases of downloading and applying state such as

```json
      "state" : "Catching up",
      "status" : [ "Catching up: downloading ledger files 20094/119803 (16%)" ]
```

#### Synced

When the node is done catching up, its state will change to

```json
      "state" : "Synced!"
```

## Logging
DigitalBits-core sends logs to standard output and `digitalbits-core.log` by default, 
configurable as `LOG_FILE_PATH`.

 Log messages are classified by progressive _priority levels_:
  `TRACE`, `DEBUG`, `INFO`, `WARNING`, `ERROR` and `FATAL`.
   The logging system only emits those messages at or above its configured logging level.

Log messages at different priority levels can be color-coded on standard output
by setting `LOG_COLOR=true` in the config file. By default they are not color-coded.

The log level can be controlled by configuration, the `-ll` command-line flag 
or adjusted dynamically by administrative (HTTP) commands. Run:

`$ digitalbits-core http-command "ll?level=debug"`

against a running system.
Log levels can also be adjusted on a partition-by-partition basis through the 
administrative interface.
 For example the history system can be set to DEBUG-level logging by running:

`$ digitalbits-core http-command "ll?level=debug&partition=history"` 

against a running system.
 The default log level is `INFO`, which is moderately verbose and should emit 
 progress messages every few seconds under normal operation.


## Monitoring and diagnostics

Information provided here can be used for both human operators and programmatic access.

### General node information
Run `$ digitalbits-core http-command 'info'`
The output will look something like

```json
{
   "info" : {
      "build" : "1.0.5-1-g7f8dc82",
      "history_failure_rate" : "0.0",
      "ledger" : {
         "age" : 4,
         "baseFee" : 100,
         "baseReserve" : 100000000,
         "closeTime" : 1624009448,
         "hash" : "97b5ac15116706543420b2b656c277c53351dcfdd68d7ba2ff38116d6b67b422",
         "maxTxSetSize" : 100,
         "num" : 1077364,
         "version" : 15
      },
      "network" : "LiveNet Global DigitalBits Network ; February 2021",
      "peers" : {
         "authenticated_count" : 11,
         "pending_count" : 0
      },
      "protocol_version" : 15,
      "quorum" : {
         "node" : "self",
         "qset" : {
            "agree" : 12,
            "cost" : 68552,
            "delayed" : 0,
            "disagree" : 0,
            "fail_at" : 3,
            "hash" : "bd50f0",
            "lag_ms" : 115,
            "ledger" : 1077363,
            "missing" : 0,
            "phase" : "EXTERNALIZE",
            "validated" : true
         },
         "transitive" : {
            "critical" : null,
            "intersection" : true,
            "last_check_ledger" : 1026924,
            "node_count" : 12
         }
      },
      "startedOn" : "2021-06-15T07:58:26Z",
      "state" : "Synced!"
   }
}
```

Some notable fields in `info` are:

  * `build` is the build number for this digitalbits-core instance
  * `ledger` represents the local state of your node, it may be different from the network state if your node was disconnected from the network for example. Some important sub-fields:
    * `age` : time elapsed since this ledger closed (during normal operation less than 10 seconds)
    * `num` : ledger number
    * `version` : protocol version supported by this ledger
  * `network` is the network passphrase that this core instance is connecting to
  * `peers` gives information on the connectivity to the network
    * `authenticated_count` are live connections
    * `pending_count` are connections that are not fully established yet
  * `protocol_version` is the maximum version of the protocol that this instance recognizes
  * `state` : indicates the node's synchronization status relative to the network.
  * `quorum` : summarizes the state of the DCP protocol participants, the same as the information returned by the `quorum` command (see below).

### Overlay information

The `peers` command returns information on the peers the instance is connected to.

This list is the result of both inbound connections from other peers and outbound connections from this node to other peers.

`$ digitalbits-core http-command 'peers'`

```json
{
   "authenticated_peers" : {
      "inbound" : [
         {
            "address" : "3.97.5.252:11625",
            "elapsed" : 265799,
            "id" : "can",
            "latency" : 93,
            "olver" : 15,
            "ver" : "1.0.5-1-g7f8dc82"
         },
         {
            "address" : "35.181.0.80:11625",
            "elapsed" : 265801,
            "id" : "fra-1",
            "latency" : 11,
            "olver" : 15,
            "ver" : "1.0.5-1-g7f8dc82"
         },
         {
            "address" : "13.49.180.62:11625",
            "elapsed" : 265800,
            "id" : "swe",
            "latency" : 24,
            "olver" : 15,
            "ver" : "1.0.5-1-g7f8dc82"
         },
         {
            "address" : "54.232.68.234:11625",
            "elapsed" : 265799,
            "id" : "bra",
            "latency" : 206,
            "olver" : 15,
            "ver" : "1.0.5-1-g7f8dc82"
         },
         {
            "address" : "52.63.51.186:11625",
            "elapsed" : 265798,
            "id" : "aus",
            "latency" : 289,
            "olver" : 15,
            "ver" : "1.0.5-1-g7f8dc82"
         },
         {
            "address" : "54.195.58.142:11625",
            "elapsed" : 265799,
            "id" : "irl",
            "latency" : 23,
            "olver" : 15,
            "ver" : "1.0.5-1-g7f8dc82"
         },
         {
            "address" : "35.177.74.117:11625",
            "elapsed" : 265800,
            "id" : "gbr-1",
            "latency" : 17,
            "olver" : 15,
            "ver" : "1.0.5-1-g7f8dc82"
         }
      ],
      "outbound" : [
         {
            "address" : "176.34.117.74:11625",
            "elapsed" : 265799,
            "id" : "irl-1",
            "latency" : 26,
            "olver" : 15,
            "ver" : "1.0.26"
         },
         {
            "address" : "54.179.231.67:11625",
            "elapsed" : 265799,
            "id" : "sgp-1",
            "latency" : 155,
            "olver" : 15,
            "ver" : "1.0.26"
         },
         {
            "address" : "13.251.98.56:11625",
            "elapsed" : 265799,
            "id" : "sgp",
            "latency" : 153,
            "olver" : 15,
            "ver" : "1.0.5-1-g7f8dc82"
         },
         {
            "address" : "3.64.96.173:11625",
            "elapsed" : 265799,
            "id" : "deu-1",
            "latency" : 1,
            "olver" : 15,
            "ver" : "1.0.5-1-g7f8dc82"
         }
      ]
   },
   "pending_peers" : {
      "inbound" : null,
      "outbound" : null
   }
}
```

#### Overlay topology survey

There is a survey mechanism in the overlay that allows a validator to request connection information from other nodes on the network. The survey can be triggered from a validator, and will flood through the network like any other message, but will request information about which other nodes each node is connected to and a brief summary of their per-connection traffic volumes.

By default, a node will relay or respond to a survey message if the message originated from a node in the receiving nodes transitive quorum. This behavior can be overridden by setting `SURVEYOR_KEYS` in the config file to a more restrictive set of nodes to relay or respond to.

##### Example survey command

In this example, we have three nodes `GB6I`, `GAWK`, and `GAKP` (we'll refer to them by the first four letters of their public keys). We will execute the commands below from `GAKP`

  1. `$ digitalbits-core http-command 'surveytopology?duration=1000&node=GB6IPEJ2NV3AKWK6OXZWPOJQ4HGRSB2ULMWBESZ5MUY6OSBUDGJOSPKD'`
  2. `$ digitalbits-core http-command 'surveytopology?duration=1000&node=GAWKRGXGM7PPZMQGUH2ATXUKMKZ5DTJHDV7UK7P4OHHA2BKSF3ZUEVWT'`
  3. `$ digitalbits-core http-command 'getsurveyresult'`

Once the responses are received, the `getsurveyresult` command will return a result like this:

```json
 {
   "backlog" : null,
   "badResponseNodes" : null,
   "surveyInProgress" : true,
   "topology" : {
      "GAWKRGXGM7PPZMQGUH2ATXUKMKZ5DTJHDV7UK7P4OHHA2BKSF3ZUEVWT" : {
         "inboundPeers" : [
            {
               "bytesRead" : 1319451500,
               "bytesWritten" : 1789787444,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1026522552,
               "duplicateFloodMessageRecv" : 2580699,
               "messagesRead" : 3044942,
               "messagesWritten" : 4087393,
               "nodeId" : "GBKW3R3APTMYSCZDRYNG6CCAEHDW4UNLEQQHTHL7UEFFJAWTSJOWH5Q7",
               "secondsConnected" : 266135,
               "uniqueFetchBytesRecv" : 0,
               "uniqueFetchMessageRecv" : 0,
               "uniqueFloodBytesRecv" : 142722388,
               "uniqueFloodMessageRecv" : 357679,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 1770594740,
               "bytesWritten" : 1804861344,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1441716008,
               "duplicateFloodMessageRecv" : 3608484,
               "messagesRead" : 4042683,
               "messagesWritten" : 4120575,
               "nodeId" : "GBF7CE3PPXKAWVZ255FDO5ZKEHZDBRW7SBDKBHSQZDQ3E5TRSKUSBKGT",
               "secondsConnected" : 266136,
               "uniqueFetchBytesRecv" : 0,
               "uniqueFetchMessageRecv" : 0,
               "uniqueFloodBytesRecv" : 130784724,
               "uniqueFloodMessageRecv" : 327750,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 1718480180,
               "bytesWritten" : 1806090716,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1392254840,
               "duplicateFloodMessageRecv" : 3488651,
               "messagesRead" : 3927071,
               "messagesWritten" : 4125749,
               "nodeId" : "GDNJGMQCWXN2DAPIR5NBJS775LIKYSSY35S2CSRHLDFE7NCSQNWZ2KIZ",
               "secondsConnected" : 266135,
               "uniqueFetchBytesRecv" : 120,
               "uniqueFetchMessageRecv" : 3,
               "uniqueFloodBytesRecv" : 133680340,
               "uniqueFloodMessageRecv" : 331961,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 1829936832,
               "bytesWritten" : 1850089360,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1516913072,
               "duplicateFloodMessageRecv" : 3793905,
               "messagesRead" : 4173890,
               "messagesWritten" : 4219942,
               "nodeId" : "GB4UPA2VRNJGE7EWPKCE4EQRXVOFPCVBERMXCA3ZOFJU3JU7COA7HIWG",
               "secondsConnected" : 266135,
               "uniqueFetchBytesRecv" : 40,
               "uniqueFetchMessageRecv" : 1,
               "uniqueFloodBytesRecv" : 108631064,
               "uniqueFloodMessageRecv" : 273515,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 1763768960,
               "bytesWritten" : 1726508056,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1365434196,
               "duplicateFloodMessageRecv" : 3414468,
               "messagesRead" : 4028641,
               "messagesWritten" : 3941813,
               "nodeId" : "GBDWWMQKFO3WBTSZ74F64LNXETXBD7VYQT6MIXFVIBHLM57HIR7LYKI2",
               "secondsConnected" : 266136,
               "uniqueFetchBytesRecv" : 80,
               "uniqueFetchMessageRecv" : 2,
               "uniqueFloodBytesRecv" : 200915788,
               "uniqueFloodMessageRecv" : 507751,
               "version" : "1.0.5-1-g7f8dc82"
            }
         ],
         "numTotalInboundPeers" : 5,
         "numTotalOutboundPeers" : 6,
         "outboundPeers" : [
            {
               "bytesRead" : 1792071236,
               "bytesWritten" : 1778194844,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1433378664,
               "duplicateFloodMessageRecv" : 3594717,
               "messagesRead" : 4099502,
               "messagesWritten" : 4059217,
               "nodeId" : "GDKMIZ6AJQVGYIKFNXLL6DR3J2V252ZVNIKMX5R4MCN4A567ESURCRZJ",
               "secondsConnected" : 266135,
               "uniqueFetchBytesRecv" : 80,
               "uniqueFetchMessageRecv" : 2,
               "uniqueFloodBytesRecv" : 157870864,
               "uniqueFloodMessageRecv" : 398329,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 1809998112,
               "bytesWritten" : 1760959280,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1430406932,
               "duplicateFloodMessageRecv" : 3586460,
               "messagesRead" : 4136609,
               "messagesWritten" : 4023246,
               "nodeId" : "GAKPT7BFXX224DJ7KB7V22LTJ6WH4SRQSJ3VLW324FIVFB2P6VW2OF76",
               "secondsConnected" : 266087,
               "uniqueFetchBytesRecv" : 0,
               "uniqueFetchMessageRecv" : 0,
               "uniqueFloodBytesRecv" : 176989172,
               "uniqueFloodMessageRecv" : 443715,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 1699440284,
               "bytesWritten" : 1808372100,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1381874900,
               "duplicateFloodMessageRecv" : 3457222,
               "messagesRead" : 3879126,
               "messagesWritten" : 4132485,
               "nodeId" : "GB6IPEJ2NV3AKWK6OXZWPOJQ4HGRSB2ULMWBESZ5MUY6OSBUDGJOSPKD",
               "secondsConnected" : 266135,
               "uniqueFetchBytesRecv" : 0,
               "uniqueFetchMessageRecv" : 0,
               "uniqueFloodBytesRecv" : 127319572,
               "uniqueFloodMessageRecv" : 315388,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 1810887204,
               "bytesWritten" : 1706255056,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1349137536,
               "duplicateFloodMessageRecv" : 3388373,
               "messagesRead" : 4139121,
               "messagesWritten" : 3907274,
               "nodeId" : "GAD3IYRUDJN7AVE4VUUQQO74AWFKLEFKB5BFUNOFM6KA4WH5G23GUQ7W",
               "secondsConnected" : 266135,
               "uniqueFetchBytesRecv" : 40,
               "uniqueFetchMessageRecv" : 1,
               "uniqueFloodBytesRecv" : 259026368,
               "uniqueFloodMessageRecv" : 644295,
               "version" : "1.0.26"
            },
            {
               "bytesRead" : 1866721248,
               "bytesWritten" : 1873733952,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1563607960,
               "duplicateFloodMessageRecv" : 3913744,
               "messagesRead" : 4257175,
               "messagesWritten" : 4274491,
               "nodeId" : "GAH63EU4HJANIP3W6UNCJ2YKOYRZQJHYWQBKZGXVZK6UFNQ4SULCKWLC",
               "secondsConnected" : 266135,
               "uniqueFetchBytesRecv" : 40,
               "uniqueFetchMessageRecv" : 1,
               "uniqueFloodBytesRecv" : 94722784,
               "uniqueFloodMessageRecv" : 236961,
               "version" : "1.0.26"
            },
            {
               "bytesRead" : 1747241704,
               "bytesWritten" : 1761972356,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1378008000,
               "duplicateFloodMessageRecv" : 3445213,
               "messagesRead" : 3990007,
               "messagesWritten" : 4023986,
               "nodeId" : "GDS25FEPPK5LK5BVWGEPLFCQQV7DQOXS6ERYWHDQIKZU3YGO5NRODIAT",
               "secondsConnected" : 266135,
               "uniqueFetchBytesRecv" : 120,
               "uniqueFetchMessageRecv" : 3,
               "uniqueFloodBytesRecv" : 173669564,
               "uniqueFloodMessageRecv" : 438386,
               "version" : "1.0.5-1-g7f8dc82"
            }
         ]
      },
      "GB6IPEJ2NV3AKWK6OXZWPOJQ4HGRSB2ULMWBESZ5MUY6OSBUDGJOSPKD" : {
         "inboundPeers" : [
            {
               "bytesRead" : 1987104652,
               "bytesWritten" : 1936802228,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1604039460,
               "duplicateFloodMessageRecv" : 4021363,
               "messagesRead" : 4526782,
               "messagesWritten" : 4417042,
               "nodeId" : "GBAFYZTWV5QJSCNKJ4MGMUVKB7Z4QYVJ46RU2OEQENFBXC42EFOEAG5K",
               "secondsConnected" : 266281,
               "uniqueFetchBytesRecv" : 0,
               "uniqueFetchMessageRecv" : 0,
               "uniqueFloodBytesRecv" : 161732672,
               "uniqueFloodMessageRecv" : 398923,
               "version" : "1.0.26"
            },
            {
               "bytesRead" : 1828798132,
               "bytesWritten" : 1717513652,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1378742404,
               "duplicateFloodMessageRecv" : 3458271,
               "messagesRead" : 4179287,
               "messagesWritten" : 3930517,
               "nodeId" : "GAH63EU4HJANIP3W6UNCJ2YKOYRZQJHYWQBKZGXVZK6UFNQ4SULCKWLC",
               "secondsConnected" : 267925,
               "uniqueFetchBytesRecv" : 1480,
               "uniqueFetchMessageRecv" : 37,
               "uniqueFloodBytesRecv" : 245376064,
               "uniqueFloodMessageRecv" : 613814,
               "version" : "1.0.26"
            },
            {
               "bytesRead" : 1808043016,
               "bytesWritten" : 1699120888,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1380679068,
               "duplicateFloodMessageRecv" : 3454742,
               "messagesRead" : 4131739,
               "messagesWritten" : 3878409,
               "nodeId" : "GAWKRGXGM7PPZMQGUH2ATXUKMKZ5DTJHDV7UK7P4OHHA2BKSF3ZUEVWT",
               "secondsConnected" : 266089,
               "uniqueFetchBytesRecv" : 2600,
               "uniqueFetchMessageRecv" : 65,
               "uniqueFloodBytesRecv" : 224993316,
               "uniqueFloodMessageRecv" : 570500,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 1933618092,
               "bytesWritten" : 1940615152,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1696995132,
               "duplicateFloodMessageRecv" : 4260138,
               "messagesRead" : 4414218,
               "messagesWritten" : 4429410,
               "nodeId" : "GBF7CE3PPXKAWVZ255FDO5ZKEHZDBRW7SBDKBHSQZDQ3E5TRSKUSBKGT",
               "secondsConnected" : 267925,
               "uniqueFetchBytesRecv" : 40,
               "uniqueFetchMessageRecv" : 1,
               "uniqueFloodBytesRecv" : 20668488,
               "uniqueFloodMessageRecv" : 46926,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 1859937040,
               "bytesWritten" : 980896344,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 722600940,
               "duplicateFloodMessageRecv" : 1807789,
               "messagesRead" : 4243324,
               "messagesWritten" : 2276808,
               "nodeId" : "GB4UPA2VRNJGE7EWPKCE4EQRXVOFPCVBERMXCA3ZOFJU3JU7COA7HIWG",
               "secondsConnected" : 266279,
               "uniqueFetchBytesRecv" : 4080,
               "uniqueFetchMessageRecv" : 102,
               "uniqueFloodBytesRecv" : 929604984,
               "uniqueFloodMessageRecv" : 2328924,
               "version" : "1.0.5-1-g7f8dc82"
            }
         ],
         "numTotalInboundPeers" : 5,
         "numTotalOutboundPeers" : 7,
         "outboundPeers" : [
            {
               "bytesRead" : 1959804648,
               "bytesWritten" : 1949180596,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1723596416,
               "duplicateFloodMessageRecv" : 4319460,
               "messagesRead" : 4468498,
               "messagesWritten" : 4446343,
               "nodeId" : "GBDWWMQKFO3WBTSZ74F64LNXETXBD7VYQT6MIXFVIBHLM57HIR7LYKI2",
               "secondsConnected" : 267908,
               "uniqueFetchBytesRecv" : 320,
               "uniqueFetchMessageRecv" : 8,
               "uniqueFloodBytesRecv" : 17649000,
               "uniqueFloodMessageRecv" : 41906,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 1913329324,
               "bytesWritten" : 1875729308,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1604502564,
               "duplicateFloodMessageRecv" : 4013182,
               "messagesRead" : 4364877,
               "messagesWritten" : 4273747,
               "nodeId" : "GDNJGMQCWXN2DAPIR5NBJS775LIKYSSY35S2CSRHLDFE7NCSQNWZ2KIZ",
               "secondsConnected" : 267821,
               "uniqueFetchBytesRecv" : 1920,
               "uniqueFetchMessageRecv" : 48,
               "uniqueFloodBytesRecv" : 95239720,
               "uniqueFloodMessageRecv" : 244522,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 463614892,
               "bytesWritten" : 461851692,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 402087768,
               "duplicateFloodMessageRecv" : 1009114,
               "messagesRead" : 1058538,
               "messagesWritten" : 1054304,
               "nodeId" : "GDS25FEPPK5LK5BVWGEPLFCQQV7DQOXS6ERYWHDQIKZU3YGO5NRODIAT",
               "secondsConnected" : 64428,
               "uniqueFetchBytesRecv" : 280,
               "uniqueFetchMessageRecv" : 7,
               "uniqueFloodBytesRecv" : 9738280,
               "uniqueFloodMessageRecv" : 23668,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 1891787976,
               "bytesWritten" : 1893719264,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1630266516,
               "duplicateFloodMessageRecv" : 4080218,
               "messagesRead" : 4313519,
               "messagesWritten" : 4319330,
               "nodeId" : "GBKW3R3APTMYSCZDRYNG6CCAEHDW4UNLEQQHTHL7UEFFJAWTSJOWH5Q7",
               "secondsConnected" : 266198,
               "uniqueFetchBytesRecv" : 40,
               "uniqueFetchMessageRecv" : 1,
               "uniqueFloodBytesRecv" : 50426624,
               "uniqueFloodMessageRecv" : 126837,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 1947961272,
               "bytesWritten" : 1938537620,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1719576312,
               "duplicateFloodMessageRecv" : 4310221,
               "messagesRead" : 4441329,
               "messagesWritten" : 4422691,
               "nodeId" : "GAKPT7BFXX224DJ7KB7V22LTJ6WH4SRQSJ3VLW324FIVFB2P6VW2OF76",
               "secondsConnected" : 266042,
               "uniqueFetchBytesRecv" : 440,
               "uniqueFetchMessageRecv" : 11,
               "uniqueFloodBytesRecv" : 11156796,
               "uniqueFloodMessageRecv" : 24685,
               "version" : "1.0.5-1-g7f8dc82"
            },
            {
               "bytesRead" : 1927832452,
               "bytesWritten" : 1891387552,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1635660264,
               "duplicateFloodMessageRecv" : 4092621,
               "messagesRead" : 4396609,
               "messagesWritten" : 4311492,
               "nodeId" : "GAD3IYRUDJN7AVE4VUUQQO74AWFKLEFKB5BFUNOFM6KA4WH5G23GUQ7W",
               "secondsConnected" : 267924,
               "uniqueFetchBytesRecv" : 1200,
               "uniqueFetchMessageRecv" : 30,
               "uniqueFloodBytesRecv" : 77061364,
               "uniqueFloodMessageRecv" : 196797,
               "version" : "1.0.26"
            },
            {
               "bytesRead" : 1960772888,
               "bytesWritten" : 1953499612,
               "duplicateFetchBytesRecv" : 0,
               "duplicateFetchMessageRecv" : 0,
               "duplicateFloodBytesRecv" : 1731586832,
               "duplicateFloodMessageRecv" : 4338556,
               "messagesRead" : 4470686,
               "messagesWritten" : 4455365,
               "nodeId" : "GDKMIZ6AJQVGYIKFNXLL6DR3J2V252ZVNIKMX5R4MCN4A567ESURCRZJ",
               "secondsConnected" : 267924,
               "uniqueFetchBytesRecv" : 200,
               "uniqueFetchMessageRecv" : 5,
               "uniqueFloodBytesRecv" : 10520348,
               "uniqueFloodMessageRecv" : 24959,
               "version" : "1.0.5-1-g7f8dc82"
            }
         ]
      }
   }
}
```

Notable field definitions

* `backlog` : List of nodes for which the survey request are yet to be sent
* `badResponseNodes` : List of nodes that sent a malformed response
* `topology` : Map of nodes to connection information
  * `inboundPeers`/`outboundPeers` : List of connection information by nodes
  * `numTotalInboundPeers`/`numTotalOutboundPeers` : The number of total inbound and outbound peers this node is connected to. The response will have a random subset of 25 connected peers per direction (inbound/outbound). These fields tell you if you're missing nodes so you can send another request out to get another random subset of nodes.

### Quorum Health

#### Quorum set diagnostics
The `quorum` command allows to diagnose problems with the quorum set of the local node.

Run

`$ digitalbits-core http-command 'quorum'`

The output looks something like:

```json
 {
   "node" : "self",
   "qset" : {
      "agree" : 12,
      "cost" : {
         "1077499" : {
            "aus" : 6232,
            "bra" : 6232,
            "can" : 6232,
            "deu-1" : 6232,
            "fra-1" : 6232,
            "gbr-1" : 6232,
            "irl" : 6232,
            "irl-1" : 6232,
            "sgp" : 6232,
            "sgp-1" : 6232,
            "swe" : 6232
         },
         "1077500" : {
            "aus" : 6232,
            "bra" : 6232,
            "can" : 6232,
            "deu-1" : 6232,
            "fra-1" : 6232,
            "gbr-1" : 6232,
            "irl" : 6232,
            "irl-1" : 6232,
            "sgp" : 6232,
            "sgp-1" : 6232,
            "swe" : 6232
         },
         "1077501" : {
            "aus" : 6232,
            "bra" : 6232,
            "can" : 6232,
            "deu-1" : 6232,
            "fra-1" : 6232,
            "gbr-1" : 6232,
            "irl" : 6232,
            "irl-1" : 6232,
            "sgp" : 6232,
            "sgp-1" : 6232,
            "swe" : 6232
         },
         "1077502" : {
            "aus" : 6232,
            "bra" : 6232,
            "can" : 6232,
            "deu-1" : 6232,
            "fra-1" : 6232,
            "gbr-1" : 6232,
            "irl" : 6232,
            "irl-1" : 6232,
            "sgp" : 6232,
            "sgp-1" : 6232,
            "swe" : 6424
         },
         "1077503" : {
            "aus" : 6232,
            "bra" : 6232,
            "can" : 6232,
            "deu-1" : 6232,
            "fra-1" : 6232,
            "gbr-1" : 6424,
            "irl" : 6232,
            "irl-1" : 6232,
            "sgp" : 6232,
            "sgp-1" : 6232,
            "swe" : 6232
         },
         "1077504" : {
            "aus" : 6232,
            "bra" : 6232,
            "can" : 6232,
            "deu-1" : 6232,
            "fra-1" : 6232,
            "gbr-1" : 6232,
            "irl" : 6232,
            "irl-1" : 6232,
            "sgp" : 6424,
            "sgp-1" : 6232,
            "swe" : 6232
         },
         "1077505" : {
            "aus" : 6232,
            "bra" : 6232,
            "can" : 6232,
            "deu-1" : 6232,
            "fra-1" : 6232,
            "gbr-1" : 6232,
            "irl" : 6424,
            "irl-1" : 6232,
            "sgp" : 6232,
            "sgp-1" : 6232,
            "swe" : 6232
         },
         "1077506" : {
            "aus" : 6424,
            "bra" : 6232,
            "can" : 6232,
            "deu-1" : 6232,
            "fra-1" : 6232,
            "gbr-1" : 6232,
            "irl" : 6232,
            "irl-1" : 6424,
            "sgp" : 6232,
            "sgp-1" : 6232,
            "swe" : 6232
         },
         "1077507" : {
            "aus" : 6232,
            "bra" : 6232,
            "can" : 6232,
            "deu-1" : 6232,
            "fra-1" : 6232,
            "gbr-1" : 6232,
            "irl" : 6232,
            "irl-1" : 6232,
            "sgp" : 6232,
            "sgp-1" : 6232,
            "swe" : 6232
         },
         "1077508" : {
            "aus" : 6232,
            "bra" : 6232,
            "can" : 6232,
            "deu-1" : 6232,
            "fra-1" : 6232,
            "gbr-1" : 6232,
            "irl" : 6232,
            "irl-1" : 6232,
            "sgp" : 6232,
            "sgp-1" : 6232,
            "swe" : 6232
         },
         "1077509" : {
            "aus" : 6232,
            "bra" : 6232,
            "can" : 6232,
            "deu-1" : 6232,
            "fra-1" : 6232,
            "gbr-1" : 6232,
            "irl" : 6232,
            "irl-1" : 6232,
            "sgp" : 6232,
            "sgp-1" : 6232,
            "swe" : 6232
         },
         "1077510" : {
            "aus" : 6232,
            "bra" : 6232,
            "can" : 6232,
            "deu-1" : 6232,
            "fra-1" : 6424,
            "gbr-1" : 6232,
            "irl" : 6232,
            "irl-1" : 6232,
            "sgp" : 6232,
            "sgp-1" : 6232,
            "swe" : 6232
         }
      },
      "delayed" : null,
      "disagree" : null,
      "fail_at" : 3,
      "fail_with" : [ "irl-1", "sgp-1", "fra-1" ],
      "hash" : "bd50f0",
      "lag_ms" : {
         "aus" : 275,
         "bra" : 198,
         "can" : 96,
         "deu-1" : 44,
         "fra-1" : 62,
         "gbr-1" : 51,
         "irl" : 46,
         "irl-1" : 76,
         "sgp" : 159,
         "sgp-1" : 187,
         "swe" : 86
      },
      "ledger" : 1077510,
      "missing" : null,
      "phase" : "EXTERNALIZE",
      "validated" : true,
      "value" : {
         "t" : 2,
         "v" : [
            {
               "t" : 3,
               "v" : [ "irl-1", "sgp-1", "fra-1", "deu-1", "gbr-1" ]
            },
            {
               "t" : 4,
               "v" : [ "self", "can", "swe", "bra", "sgp", "aus", "irl" ]
            }
         ]
      }
   },
   "transitive" : {
      "critical" : null,
      "intersection" : true,
      "last_check_ledger" : 1026924,
      "node_count" : 12
   }
}
```

This output has two main sections: `qset` and `transitive`. The former describes the node and its quorum set. The latter describes the transitive closure of the node's quorum set.

##### Per-node quorum-set information

Entries to watch for in the `qset` section -- describing the node and its quorum set -- are:

  * `agree` : the number of nodes in the quorum set that agree with this instance.
  * `delayed` : the nodes that are participating to consensus but seem to be behind.
  * `disagree`: the nodes that were participating but disagreed with this instance.
  * `fail_at` : the number of failed nodes that *would* cause this instance to halt.
  * `fail_with`: an example of such potential failure.
  * `missing` : the nodes that were missing during this consensus round.
  * `value` : the quorum set used by this node (`t` is the threshold expressed as a number of nodes).

In the example above, 6 nodes are functioning properly, one is down (`stronghold1`), and
 the instance will fail if any two nodes still working (or one node and one inner-quorum-set) fail as well.

If a node is stuck in state `Joining DCP`, this command allows to quickly find the reason:

  * too many validators missing (down or without a good connectivity), solutions are:
    * [adjust quorum set](#crafting-a-quorum-set) (thresholds, grouping, etc) based on the nodes that are not missing
    * try to get a [better connectivity path](#quorum-and-overlay-network) to the missing validators
  * network split would cause DCP to be stuck because of nodes that disagree. This would happen if either there is a bug in DCP, the network does not have quorum intersection or the disagreeing nodes are misbehaving (compromised, etc)

Note that the node not being able to reach consensus does not mean that the network
as a whole will not be able to reach consensus (and the opposite is true, the network
may fail because of a different set of validators failing).

You can get a sense of the quorum set health of a different node by doing
`$ digitalbits-core http-command 'quorum?node=$deu'` or `$ digitalbits-core http-command 'quorum?node=@GDKMIZ` 

Overall network health can be evaluated by walking through all nodes and looking at their health. Note that this is only an approximation as remote nodes may not have received the same messages (in particular: `missing` for 
other nodes is not reliable).

##### Transitive closure summary information

When showing quorum-set information about the local node rather than some other
node, a summary of the transitive closure of the quorum set is also provided in
the `transitive` field. This has several important sub-fields:

  * `last_check_ledger` : the last ledger in which the transitive closure was checked for quorum intersection. This will reset when the node boots and whenever a node in the transitive quorum changes its quorum set. It may lag behind the last-closed ledger by a few ledgers depending on the computational cost of checking quorum intersection.
  * `node_count` : the number of nodes in the transitive closure, which are considered when calculating quorum intersection.
  * `intersection` : whether or not the transitive closure enjoyed quorum intersection at the most recent check. This is of **utmost importance** in preventing network splits. It should always be true. If it is ever false, one or more nodes in the transitive closure of the quorum set is _currently_ misconfigured, and the network is at risk of splitting. Corrective action should be taken immediately, for which two additional sub-fields will be present to help suggest remedies:
    * `last_good_ledger` : this will note the last ledger for which the `intersection` field was evaluated as true; if some node reconfigured at or around that ledger, reverting that configuration change is the easiest corrective action to take.
    * `potential_split` : this will contain a pair of lists of validator IDs, which is a potential pair of disjoint quorums that allowed by the current configuration. In other words, a possible split in consensus allowed by the current configuration. This may help narrow down the cause of the misconfiguration: likely the misconfiguration involves too-low a consensus threshold in one of the two potential quorums, and/or the absence of a mandatory trust relationship that would bridge the two.
  * `critical`: an "advance warning" field that lists nodes that _could cause_ the network to fail to enjoy quorum intersection, if they were misconfigured sufficiently badly. In a healthy transitive network configuration, this field will be `null`. If it is non-`null` then the network is essentially "one misconfiguration" (of the quorum sets of the listed nodes) away from no longer enjoying quorum intersection, and again, corrective action should be taken: careful adjustment to the quorum sets of _nodes that depend on_ the listed nodes, typically to strengthen quorums that depend on them.

#### Detailed transitive quorum analysis

The quorum endpoint can also retrieve detailed information for the transitive quorum.

This is an easier to process format than what `dcp` returns as it doesn't contain all DCP messages.

`$ digitalbits-core http-command 'quorum?transitive=true'`

The output looks something like:

```json
{
   "critical" : null,
   "intersection" : true,
   "last_check_ledger" : 1026924,
   "node_count" : 12,
   "nodes" : [
      {
         "distance" : 0,
         "heard" : 1077537,
         "node" : "self",
         "qset" : {
            "t" : 2,
            "v" : [
               {
                  "t" : 3,
                  "v" : [ "irl-1", "sgp-1", "fra-1", "deu-1", "gbr-1" ]
               },
               {
                  "t" : 4,
                  "v" : [ "self", "can", "swe", "bra", "sgp", "aus", "irl" ]
               }
            ]
         },
         "status" : "tracking",
         "value" : "[ SIGNED@irl-1 txH: cb7bcc, ct: 1624010357, upgrades: [ ] ]",
         "value_id" : 1
      },
      {
         "distance" : 1,
         "heard" : 1077537,
         "node" : "irl-1",
         "qset" : {
            "t" : 2,
            "v" : [
               {
                  "t" : 3,
                  "v" : [ "irl-1", "sgp-1", "fra-1", "deu-1", "gbr-1" ]
               },
               {
                  "t" : 4,
                  "v" : [ "self", "can", "swe", "bra", "sgp", "aus", "irl" ]
               }
            ]
         },
         "status" : "tracking",
         "value" : "[ SIGNED@irl-1 txH: cb7bcc, ct: 1624010357, upgrades: [ ] ]",
         "value_id" : 1
      },
      {
         "distance" : 1,
         "heard" : 1077537,
         "node" : "sgp-1",
         "qset" : {
            "t" : 2,
            "v" : [
               {
                  "t" : 3,
                  "v" : [ "irl-1", "sgp-1", "fra-1", "deu-1", "gbr-1" ]
               },
               {
                  "t" : 4,
                  "v" : [ "self", "can", "swe", "bra", "sgp", "aus", "irl" ]
               }
            ]
         },
         "status" : "tracking",
         "value" : "[ SIGNED@irl-1 txH: cb7bcc, ct: 1624010357, upgrades: [ ] ]",
         "value_id" : 1
      },
      {
         "distance" : 1,
         "heard" : 1077537,
         "node" : "can",
         "qset" : {
            "t" : 2,
            "v" : [
               {
                  "t" : 3,
                  "v" : [ "irl-1", "sgp-1", "fra-1", "deu-1", "gbr-1" ]
               },
               {
                  "t" : 4,
                  "v" : [ "self", "can", "swe", "bra", "sgp", "aus", "irl" ]
               }
            ]
         },
         "status" : "tracking",
         "value" : "[ SIGNED@irl-1 txH: cb7bcc, ct: 1624010357, upgrades: [ ] ]",
         "value_id" : 1
      },
      {
         "distance" : 1,
         "heard" : 1077537,
         "node" : "fra-1",
         "qset" : {
            "t" : 2,
            "v" : [
               {
                  "t" : 3,
                  "v" : [ "irl-1", "sgp-1", "fra-1", "deu-1", "gbr-1" ]
               },
               {
                  "t" : 4,
                  "v" : [ "self", "can", "swe", "bra", "sgp", "aus", "irl" ]
               }
            ]
         },
         "status" : "tracking",
         "value" : "[ SIGNED@irl-1 txH: cb7bcc, ct: 1624010357, upgrades: [ ] ]",
         "value_id" : 1
      },
      {
         "distance" : 1,
         "heard" : 1077537,
         "node" : "swe",
         "qset" : {
            "t" : 2,
            "v" : [
               {
                  "t" : 3,
                  "v" : [ "irl-1", "sgp-1", "fra-1", "deu-1", "gbr-1" ]
               },
               {
                  "t" : 4,
                  "v" : [ "self", "can", "swe", "bra", "sgp", "aus", "irl" ]
               }
            ]
         },
         "status" : "tracking",
         "value" : "[ SIGNED@irl-1 txH: cb7bcc, ct: 1624010357, upgrades: [ ] ]",
         "value_id" : 1
      },
      {
         "distance" : 1,
         "heard" : 1077537,
         "node" : "bra",
         "qset" : {
            "t" : 2,
            "v" : [
               {
                  "t" : 3,
                  "v" : [ "irl-1", "sgp-1", "fra-1", "deu-1", "gbr-1" ]
               },
               {
                  "t" : 4,
                  "v" : [ "self", "can", "swe", "bra", "sgp", "aus", "irl" ]
               }
            ]
         },
         "status" : "tracking",
         "value" : "[ SIGNED@irl-1 txH: cb7bcc, ct: 1624010357, upgrades: [ ] ]",
         "value_id" : 1
      },
      {
         "distance" : 1,
         "heard" : 1077537,
         "node" : "sgp",
         "qset" : {
            "t" : 2,
            "v" : [
               {
                  "t" : 3,
                  "v" : [ "irl-1", "sgp-1", "fra-1", "deu-1", "gbr-1" ]
               },
               {
                  "t" : 4,
                  "v" : [ "self", "can", "swe", "bra", "sgp", "aus", "irl" ]
               }
            ]
         },
         "status" : "tracking",
         "value" : "[ SIGNED@irl-1 txH: cb7bcc, ct: 1624010357, upgrades: [ ] ]",
         "value_id" : 1
      },
      {
         "distance" : 1,
         "heard" : 1077537,
         "node" : "aus",
         "qset" : {
            "t" : 2,
            "v" : [
               {
                  "t" : 3,
                  "v" : [ "irl-1", "sgp-1", "fra-1", "deu-1", "gbr-1" ]
               },
               {
                  "t" : 4,
                  "v" : [ "self", "can", "swe", "bra", "sgp", "aus", "irl" ]
               }
            ]
         },
         "status" : "tracking",
         "value" : "[ SIGNED@irl-1 txH: cb7bcc, ct: 1624010357, upgrades: [ ] ]",
         "value_id" : 1
      },
      {
         "distance" : 1,
         "heard" : 1077537,
         "node" : "deu-1",
         "qset" : {
            "t" : 2,
            "v" : [
               {
                  "t" : 3,
                  "v" : [ "irl-1", "sgp-1", "fra-1", "deu-1", "gbr-1" ]
               },
               {
                  "t" : 4,
                  "v" : [ "self", "can", "swe", "bra", "sgp", "aus", "irl" ]
               }
            ]
         },
         "status" : "tracking",
         "value" : "[ SIGNED@irl-1 txH: cb7bcc, ct: 1624010357, upgrades: [ ] ]",
         "value_id" : 1
      },
      {
         "distance" : 1,
         "heard" : 1077537,
         "node" : "irl",
         "qset" : {
            "t" : 2,
            "v" : [
               {
                  "t" : 3,
                  "v" : [ "irl-1", "sgp-1", "fra-1", "deu-1", "gbr-1" ]
               },
               {
                  "t" : 4,
                  "v" : [ "self", "can", "swe", "bra", "sgp", "aus", "irl" ]
               }
            ]
         },
         "status" : "tracking",
         "value" : "[ SIGNED@irl-1 txH: cb7bcc, ct: 1624010357, upgrades: [ ] ]",
         "value_id" : 1
      },
      {
         "distance" : 1,
         "heard" : 1077537,
         "node" : "gbr-1",
         "qset" : {
            "t" : 2,
            "v" : [
               {
                  "t" : 3,
                  "v" : [ "irl-1", "sgp-1", "fra-1", "deu-1", "gbr-1" ]
               },
               {
                  "t" : 4,
                  "v" : [ "self", "can", "swe", "bra", "sgp", "aus", "irl" ]
               }
            ]
         },
         "status" : "tracking",
         "value" : "[ SIGNED@irl-1 txH: cb7bcc, ct: 1624010357, upgrades: [ ] ]",
         "value_id" : 1
      }
   ]
}
```

The output begins with the same summary information as in the `transitive` block
of the non-transitive query (if queried for the local node), but also includes
a `nodes` array that represents a walk of the transitive quorum centered on
the query node.

Fields are:

* `node` : the identity of the validator
* `distance` : how far that node is from the root node (ie. how many quorum set hops)
* `heard` : the latest ledger sequence number that this node voted at
* `qset` : the node's quorum set
* `status` : one of `behind|tracking|ahead` (compared to the root node) or `missing|unknown` (when there are no recent SCP messages for that node)
* `value_id` : a unique ID for what the node is voting for (allows to quickly tell if nodes are voting for the same thing)
* `value` : what the node is voting for

## Validator maintenance

Maintenance here refers to anything involving taking your validator temporarily out of the network (to apply security patches, system upgrade, etc).

As an administrator of a validator, you must ensure that the maintenance you are about to apply to the validator is safe for the overall network and for your validator.

Safe means that the other validators that depend on yours will not be affected too much when you turn off your validator for maintenance and that your validator will continue to operate as part of the network when it comes back up.

If you are changing some settings that may impact network wide settings such as protocol version, review [the section on network configuration](#network-configuration).

If you're changing your quorum set configuration, also read the [section on what to do](#special-considerations-during-quorum-set-updates).

### Recommended steps to perform as part of a maintenance

We recommend performing the following steps in order (repeat sequentially as needed if you run multiple nodes).

  1. Advertise your intention to others that may depend on you. Some coordination is required to avoid situations where too many nodes go down at the same time.
  2. Dependencies should assess the health of their quorum, refer to the section
     "Understanding quorum and reliability".
  3. If there is no objection, take your instance down
  4. When done, start your instance that should rejoin the network
  5. The instance will be completely caught up when it's both `Synced` and *there is no backlog in uploading history*.

## Network configuration

The network itself has network wide settings that can be updated.

This is performed by validators voting for and agreeing to new values the same way than consensus is reached for transaction sets, etc.

A node can be configured to vote for upgrades using the `upgrades` endpoint . see [`commands.md`](commands.md) for more information.

The network settings are:

  * the version of the protocol used to process transactions
  * the maximum number of operations that can be included in a given ledger close
  * the cost (fee) associated with processing operations
  * the base reserve used to calculate the digitalbit balance needed to store things in the ledger

When the network time is later than the `upgradetime` specified in
the upgrade settings, the validator will vote to update the network
to the value specified in the upgrade setting. If the network time 
is passed the `upgradetime` by more than 12 hours, the upgrade will be ignored

When a validator is armed to change network values, the output of `info` will contain information about the vote.

For a new value to be adopted, the same level of consensus between nodes needs to be reached as for transaction sets.

### Important notes on network wide settings

Changes to network wide settings have to be orchestrated properly between
validators as well as non validating nodes:

  * a change is vetted between operators (changes can be bundled)
  * an effective date in the future is picked for the change to take effect (controlled by `upgradetime`)
  * if applicable, communication is sent out to all network users

An improper plan may cause issues such as:

  * nodes missing consensus (aka "getting stuck"), and having to use history to rejoin
  * network reconfiguration taking effect at a non deterministic time (causing fees to change ahead of schedule for example)

For more information look at [`docs/versioning.md`](https://github.com/xdbfoundation/DigitalBits/blob/master/docs/versioning.md).

### Example upgrade command

Example here is to upgrade the protocol version to version 15 on February-28-2021.

  1. `$ digitalbits-core http-command 'upgrades?mode=set&upgradetime=2021-02-28T20:00:00Z&protocolversion=15'`
  2. `$ digitalbits-core http-command info`

At this point `info` will tell you that the node is setup to vote for this upgrade:
```json
      "status" : [
         "Armed with network upgrades: upgradetime=2021-02-28T20:00:00Z, protocolversion=15"
      ]
```

## Advanced topics and internals

This section contains information that is useful to know but that should not stop somebody from running a node.

### Creating your own private network


[testnet](./testnet.md) is a short tutorial demonstrating how to
  configure and run a short-lived, isolated test network.

### Runtime information: start and stop

DigitalBits-core can be started directly from the command line, or through a supervision 
system such as `init`, `upstart`, or `systemd`.

DigitalBits-core can be gracefully exited at any time by delivering `SIGINT` or
 pressing `CTRL-C`. It can be safely, forcibly terminated with `SIGTERM` or
  `SIGKILL`. The latter may leave a stale lock file in the `BUCKET_DIR_PATH`,
   and you may need to remove the file before it will restart. 
   Otherwise, all components are designed to recover from abrupt termination.

### In depth architecture


[architecture](https://github.com/xdbfoundation/DigitalBits/blob/master/docs/architecture.md) 

  describes how digitalbits-core is structured internally, how it is intended to be 
  deployed, and the collection of servers and services needed to get the full 
  functionality and performance.
