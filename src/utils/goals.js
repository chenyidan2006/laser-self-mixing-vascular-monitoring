export const goalDefinitions=[
  {key:'measurements',title:'Measurements',icon:'♡',unit:'times',color:'#7863ea',target:3,step:1,max:20},
  {key:'water',title:'Hydration',icon:'◉',unit:'mL',color:'#299dc8',target:2000,step:250,max:6000},
  {key:'activity',title:'Activity',icon:'↗',unit:'min',color:'#dc9554',target:30,step:5,max:180},
  {key:'sleep',title:'Sleep',icon:'☾',unit:'h',color:'#6482cb',target:8,step:0.5,max:14}
];
export function localDay(){const d=new Date();return `${d.getFullYear()}-${String(d.getMonth()+1).padStart(2,'0')}-${String(d.getDate()).padStart(2,'0')}`;}
export function readGoals(userId){
  const targets=uni.getStorageSync('dailyGoals_'+userId)||{};
  const values=uni.getStorageSync('dailyProgress_'+userId+'_'+localDay())||{};
  return {targets:Object.fromEntries(goalDefinitions.map(d=>[d.key,Number(targets[d.key])|| (d.key==='measurements'?Number(uni.getStorageSync('measurementGoal_'+userId)):0)||d.target])),values:Object.fromEntries(goalDefinitions.map(d=>[d.key,Math.max(0,Number(values[d.key])||0)]))};
}
export function saveGoals(userId,state){uni.setStorageSync('dailyGoals_'+userId,state.targets);uni.setStorageSync('dailyProgress_'+userId+'_'+localDay(),state.values);uni.setStorageSync('measurementGoal_'+userId,state.targets.measurements);}
export function goalProgress(value,target){return Math.min(100,Math.round(Math.max(0,value)/target*100));}
