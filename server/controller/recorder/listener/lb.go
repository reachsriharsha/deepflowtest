/*
 * Copyright (c) 2022 Yunshan Networks
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package listener

import (
	cloudmodel "github.com/deepflowys/deepflow/server/controller/cloud/model"
	"github.com/deepflowys/deepflow/server/controller/db/mysql"
	"github.com/deepflowys/deepflow/server/controller/recorder/cache"
	"github.com/deepflowys/deepflow/server/controller/recorder/event"
	"github.com/deepflowys/deepflow/server/libs/queue"
)

type LB struct {
	cache         *cache.Cache
	eventProducer *event.LB
}

func NewLB(c *cache.Cache, eq *queue.OverwriteQueue) *LB {
	lisener := &LB{
		cache:         c,
		eventProducer: event.NewLB(&c.ToolDataSet, eq),
	}
	return lisener
}

func (p *LB) OnUpdaterAdded(addedDBItems []*mysql.LB) {
	// p.cache.AddLBs(addedDBItems)
	p.eventProducer.ProduceByAdd(addedDBItems)
}

func (p *LB) OnUpdaterUpdated(cloudItem *cloudmodel.LB, diffBase *cache.LB) {
	p.eventProducer.ProduceByUpdate(cloudItem, diffBase)
	// diffBase.Update(cloudItem)
	// p.cache.UpdateLB(cloudItem)
}

func (p *LB) OnUpdaterDeleted(lcuuids []string) {
	p.eventProducer.ProduceByDelete(lcuuids)
	// p.cache.DeleteLBs(lcuuids)
}